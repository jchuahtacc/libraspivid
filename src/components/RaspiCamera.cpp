#include "raspivid/components/RaspiCamera.h"

namespace raspivid {
    
    void DefaultRaspiCameraCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
            MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
            switch (param->hdr.id) {
                case MMAL_PARAMETER_CAMERA_SETTINGS : {
                    MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
                    vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
                    settings->exposure,
                            settings->analog_gain.num, settings->analog_gain.den,
                            settings->digital_gain.num, settings->digital_gain.den);
                    vcos_log_error("AWB R=%u/%u, B=%u/%u",
                            settings->awb_red_gain.num, settings->awb_red_gain.den,
                            settings->awb_blue_gain.num, settings->awb_blue_gain.den);
                }
                break;
            }
        } else if (buffer->cmd == MMAL_EVENT_ERROR) {
            vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
        } else {
            vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
        }
    };

    const char* RaspiCamera::component_name() {
        return MMAL_COMPONENT_DEFAULT_CAMERA;
    }

    RASPICAMERA_OPTION_S RaspiCamera::createDefaultCameraOptions() {
        RASPICAMERA_OPTION_S options;
        options.width = 1920;
        options.height = 1080;
        options.framerate = 0;
        options.cameraNum = 0;
        options.sensor_mode = 0;
        options.settings_callback = nullptr;
        options.verbose = true;
        raspicamcontrol_set_defaults(&options.camera_parameters);
        return options;
    }

    shared_ptr< RaspiCamera > RaspiCamera::create(RASPICAMERA_OPTION_S options) {
        shared_ptr< RaspiCamera > result = shared_ptr< RaspiCamera >( new RaspiCamera() );
        result->options_ = options;
        if (result->init() != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;
    }

    shared_ptr< RaspiCamera > RaspiCamera::create() {
        return create(RaspiCamera::createDefaultCameraOptions());
    }

    MMAL_STATUS_T RaspiCamera::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, options_.cameraNum};

        if ((status = mmal_port_parameter_set(component->control, &camera_num.hdr)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::init(): could not select camera (%u)", status);
            return status;
        }

        assert_ports(0, 3);

        MMAL_PORT_T *preview_port = component->output[MMAL_CAMERA_PREVIEW_PORT];
        MMAL_PORT_T *still_port = component->output[MMAL_CAMERA_CAPTURE_PORT];
        MMAL_PORT_T *video_port = component->output[MMAL_CAMERA_VIDEO_PORT];
        

        preview = RaspiPort::create(preview_port, "RaspiCamera::preview");
        still = RaspiPort::create(still_port, "RaspiCamera::still");
        video = RaspiPort::create(video_port, "RaspiCamera::video");
        default_output = video;

        if ((status = mmal_port_parameter_set_uint32(component->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, options_.sensor_mode)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::init(): unable to set sensor mode (%u)", status);
        }

        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::init(): unable to enable camera component (%u)", status);
            return status;
        }

        if (options_.settings_callback) {
            MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
                {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
                MMAL_PARAMETER_CAMERA_SETTINGS, 1};

            status = mmal_port_parameter_set(component->control, &change_event_request.hdr);
            if ( status != MMAL_SUCCESS ) {
                vcos_log_error("RaspiCamera::init(): unable to request settings events");
            }
            component->control->userdata = (struct MMAL_PORT_USERDATA_T *)&userdata;
            userdata.cb_instance = options_.settings_callback;
            if ((status = mmal_port_enable(component->control, callback_wrapper)) != MMAL_SUCCESS) {
                vcos_log_error("RaspiCamera::init(): unable to add settings callback");
            }
        }

        //  set up the camera configuration
        {
            MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
                { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
                .max_stills_w = options_.width,
                .max_stills_h = options_.height,
                .stills_yuv422 = 0,
                .one_shot_stills = 0,
                .max_preview_video_w = options_.width,
                .max_preview_video_h = options_.height,
                .num_preview_video_frames = 3 + vcos_max(0, (options_.framerate-30)/10),
                .stills_capture_circular_buffer_height = 0,
                .fast_preview_resume = 0,
                .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC
            };
            mmal_port_parameter_set(component->control, &cam_config.hdr);
        }
        // Now set up the port formats

        // Set the encode format on the Preview port
        // HW limitations mean we need the preview to be the same size as the required recorded output

        MMAL_ES_FORMAT_T* format;
        format = preview_port->format;

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;

        if (options_.camera_parameters.shutter_speed > 6000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 50, 1000 }, {166, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
        } else if(options_.camera_parameters.shutter_speed > 1000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 166, 1000 }, {999, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
        }

        //enable dynamic framerate if necessary
        if (options_.camera_parameters.shutter_speed) {
            if (options_.framerate > 1000000./options_.camera_parameters.shutter_speed) {
                options_.framerate=0;
                if (options_.verbose) {
                    vcos_log_error("RaspiCamera::init(): Enabling dynamic frame rate to fulfill shutter speed requirements");
                }
            }
        }

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;
        format->es->video.width = VCOS_ALIGN_UP(options_.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(options_.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = options_.width;
        format->es->video.crop.height = options_.height;
        format->es->video.frame_rate.num = 15;
        format->es->video.frame_rate.den = 1;
        format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
        format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

        status = mmal_port_format_commit(preview_port);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::init(): camera preview format couldn't be set");
            return status;
        }

        // Set the encode format on the video  port

        format = video_port->format;
        format->encoding_variant = MMAL_ENCODING_I420;

        if (options_.camera_parameters.shutter_speed > 6000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 50, 1000 }, {166, 1000}};
            mmal_port_parameter_set(video_port, &fps_range.hdr);
        } else if(options_.camera_parameters.shutter_speed > 1000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 167, 1000 }, {999, 1000}};
            mmal_port_parameter_set(video_port, &fps_range.hdr);
        }

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;
        //format->encoding = MMAL_ENCODING_I420;
        format->es->video.width = VCOS_ALIGN_UP(options_.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(options_.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = options_.width;
        format->es->video.crop.height = options_.height;
        format->es->video.frame_rate.num = options_.framerate;
        format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

        status = mmal_port_format_commit(video_port);

        if (status != MMAL_SUCCESS)
        {
          vcos_log_error("camera video format couldn't be set");
          return status;
        }

        // Ensure there are enough buffers to avoid dropping frames
        if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
          video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


        // Set the encode format on the still  port

        format = still_port->format;

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;

        format->es->video.width = VCOS_ALIGN_UP(options_.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(options_.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = options_.width;
        format->es->video.crop.height = options_.height;
        format->es->video.frame_rate.num = 0;
        format->es->video.frame_rate.den = 1;

        status = mmal_port_format_commit(still_port);

        if (status != MMAL_SUCCESS)
        {
          vcos_log_error("camera still format couldn't be set");
          return status;
        }

        /* Ensure there are enough buffers to avoid dropping frames */
        if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
          still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::init(): unable to enable camera component");
            return status;
        }

        raspicamcontrol_set_all_parameters(component, &options_.camera_parameters);

        vcos_log_error("RaspiCamera::init(): success!");

        return MMAL_SUCCESS;
    }


    void RaspiCamera::callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        RASPICAMERA_USERDATA_S *userdata = (RASPICAMERA_USERDATA_S *)port->userdata;
        vcos_assert(userdata);
        userdata->cb_instance->callback(port, buffer);
        mmal_buffer_header_release(buffer);
    }

    MMAL_STATUS_T RaspiCamera::start() {
        MMAL_STATUS_T status;
        if ((status = mmal_port_parameter_set_boolean(component->output[MMAL_CAMERA_VIDEO_PORT], MMAL_PARAMETER_CAPTURE, 1)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiCamera::start(): Unable to start camera video (%u)", status);
            return status;
        }

        return MMAL_SUCCESS;
    }

}
