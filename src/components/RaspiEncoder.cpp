#include "components/RaspiEncoder.h"

namespace raspivid {
    const char* RaspiEncoder::component_name() {
        return MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER;
    }

    RASPIENCODER_OPTION_S RaspiEncoder::createDefaultEncoderOptions() {
        RASPIENCODER_OPTION_S options;
        options.encoding = MMAL_ENCODING_H264;
        options.framerate = VIDEO_FRAME_RATE_NUM;
        options.intraperiod = -1;
        options.quantisationParameter = 0;
        options.immutableInput = 1;
        options.profile = MMAL_VIDEO_PROFILE_H264_HIGH;
        options.level = MMAL_VIDEO_LEVEL_H264_4;
        options.bInlineHeaders = 0;
        options.inlineMotionVectors = 1;
        options.intra_refresh_type = MMAL_VIDEO_INTRA_REFRESH_CYCLIC;
        options.width = 1920;
        options.height = 1080;
        options.bitrate = 17000000;
        return options;
    }

    shared_ptr< RaspiEncoder > RaspiEncoder::create(RASPIENCODER_OPTION_S options) {
        shared_ptr< RaspiEncoder > result = shared_ptr< RaspiEncoder >( new RaspiEncoder() );
        result->options_ = options;
        if (result->init() != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;
    }

    shared_ptr< RaspiEncoder > RaspiEncoder::create() {
        return create(RaspiEncoder::createDefaultEncoderOptions());
    }

    MMAL_STATUS_T RaspiEncoder::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 1);

        MMAL_PORT_T *mmal_input = component->input[0];
        MMAL_PORT_T *mmal_output = component->output[0];

        input = RaspiPort::create(mmal_input, "RaspiEncoder::input");
        output = RaspiPort::create(mmal_output, "RaspiEncoder::output");
        default_input = input;
        default_output = output;

        mmal_format_copy(mmal_output->format, mmal_input->format);

        // Only supporting H264 at the moment
        mmal_output->format->encoding = options_.encoding;

        if (options_.encoding == MMAL_ENCODING_H264) {
            if (options_.level == MMAL_VIDEO_LEVEL_H264_4) {
                if (options_.bitrate > MAX_BITRATE_LEVEL4) {
                    fprintf(stderr, "RaspiEncoder::init(): Bitrate too high: Reducing to 25MBit/s\n");
                    options_.bitrate = MAX_BITRATE_LEVEL4;
                }
            } else {
                if (options_.bitrate > MAX_BITRATE_LEVEL42) {
                    fprintf(stderr, "RaspiEncoder::init(): Bitrate too high: Reducing to 62.5MBit/s\n");
                    options_.bitrate = MAX_BITRATE_LEVEL42;
                }
            }
        } else if(options_.encoding == MMAL_ENCODING_MJPEG) {
            if (options_.bitrate > MAX_BITRATE_MJPEG) {
                fprintf(stderr, "RaspiEncoder::init(): Bitrate too high: Reducing to 25MBit/s\n");
                options_.bitrate = MAX_BITRATE_MJPEG;
            }
        }

        mmal_output->format->bitrate = options_.bitrate;

        if (options_.encoding == MMAL_ENCODING_H264) {
            mmal_output->buffer_size = mmal_output->buffer_size_recommended;
        } else {
            mmal_output->buffer_size = 256<<10;
        }


        if (mmal_output->buffer_size < mmal_output->buffer_size_min) {
            mmal_output->buffer_size = mmal_output->buffer_size_min;
        }

        mmal_output->buffer_num = mmal_output->buffer_num_recommended;

        if (mmal_output->buffer_num < mmal_output->buffer_num_min) {
            mmal_output->buffer_num = mmal_output->buffer_num_min;
        }

        // We need to set the frame rate on output to 0, to ensure it gets
        // updated correctly from the input framerate when port connected
        mmal_output->format->es->video.frame_rate.num = 0;
        mmal_output->format->es->video.frame_rate.den = 1;

        // Commit the port changes to the output port
        status = mmal_port_format_commit(mmal_output);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiEncoder::init(): Unable to set format on video encoder output port");
            return status;
        }

        if (options_.encoding == MMAL_ENCODING_H264 && options_.quantisationParameter) {
            MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, options_.quantisationParameter};
            status = mmal_port_parameter_set(mmal_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("RaspiEncoder::init(): Unable to set initial QP");
                return status;
            }

            MMAL_PARAMETER_UINT32_T param2 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, options_.quantisationParameter};
            status = mmal_port_parameter_set(mmal_output, &param2.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("RaspiEncoder::init(): Unable to set min QP");
                return status;
            }

            MMAL_PARAMETER_UINT32_T param3 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, options_.quantisationParameter};
            status = mmal_port_parameter_set(mmal_output, &param3.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("RaspiEncoder::init(): Unable to set max QP");
                return status;
            }
        }

        if (options_.encoding == MMAL_ENCODING_H264) {
            MMAL_PARAMETER_VIDEO_PROFILE_T  param;
            param.hdr.id = MMAL_PARAMETER_PROFILE;
            param.hdr.size = sizeof(param);

            param.profile[0].profile = options_.profile;

            if ((VCOS_ALIGN_UP(options_.width,16) >> 4) * (VCOS_ALIGN_UP(options_.height,16) >> 4) * options_.framerate > 245760) {
                if((VCOS_ALIGN_UP(options_.width,16) >> 4) * (VCOS_ALIGN_UP(options_.height,16) >> 4) * options_.framerate <= 522240) {
                    fprintf(stderr, "RaspiEncoder::init(): Too many macroblocks/s: Increasing H264 Level to 4.2\n");
                    options_.level = MMAL_VIDEO_LEVEL_H264_42;
                } else {
                    vcos_log_error("RaspiEncoder::init(): Too many macroblocks/s requested");
                    return status;
                }
            }

            param.profile[0].level = options_.level;

            status = mmal_port_parameter_set(mmal_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("RaspiEncoder::init(): Unable to set H264 profile");
                return status;
            }
        }

        if (mmal_port_parameter_set_boolean(mmal_input, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, options_.immutableInput) != MMAL_SUCCESS) {
            vcos_log_error("RaspiEncoder::init(): Unable to set immutable input flag");
            // Continue rather than abort..
        }

        //set INLINE HEADER flag to generate SPS and PPS for every IDR if requested
        if (mmal_port_parameter_set_boolean(mmal_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, options_.bInlineHeaders) != MMAL_SUCCESS) {
            vcos_log_error("RaspiEncoder::init(): failed to set INLINE HEADER FLAG parameters");
            // Continue rather than abort..
        }

        //set INLINE VECTORS flag to request motion vector estimates
        if (options_.encoding == MMAL_ENCODING_H264 && 
                mmal_port_parameter_set_boolean(mmal_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, options_.inlineMotionVectors) != MMAL_SUCCESS) {
            vcos_log_error("RaspiEncoder::init(): failed to set INLINE VECTORS parameters");
            // Continue rather than abort..
        }

        // Adaptive intra refresh settings
        if (options_.encoding == MMAL_ENCODING_H264 && options_.intra_refresh_type != -1) {
            MMAL_PARAMETER_VIDEO_INTRA_REFRESH_T  param;
            param.hdr.id = MMAL_PARAMETER_VIDEO_INTRA_REFRESH;
            param.hdr.size = sizeof(param);

            // Get first so we don't overwrite anything unexpectedly
            status = mmal_port_parameter_get(mmal_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_warn("RaspiEncoder::init(): Unable to get existing H264 intra-refresh values. Please update your firmware");
                // Set some defaults, don't just pass random stack data
                param.air_mbs = param.air_ref = param.cir_mbs = param.pir_mbs = 0;
            }

            param.refresh_mode = options_.intra_refresh_type;

            //if (state->intra_refresh_type == MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS)
            //   param.cir_mbs = 10;

            status = mmal_port_parameter_set(mmal_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("RaspiEncoder::init(): Unable to set H264 intra-refresh values");
                return status;
            }
        }
 
        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiEncoder::init(): unable to enable encoder component (%u)", status);
            return status;
        }

        vcos_log_error("RaspiEncoder::init(): success!");

        return MMAL_SUCCESS;
    }

}
