/**
 \file RaspiEncoder.h
 */
#ifndef __RASPIENCODER_H__
#define __RASPIENCODER_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "RaspiPort.h"


#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

namespace raspivid {
      
    /**
     \brief H264 encoder parameter structure.
     */
    struct RASPIENCODER_OPTION_S {
        MMAL_FOURCC_T encoding;                                     /**< H264 encoding. */
        int bitrate;                                                /**< Desired bitrate. */
        uint32_t width;                                             /**< Frame width. */
        uint32_t height;                                            /**< Frame height. */
        uint32_t framerate;                                         /**< Desired framerate. */
        uint32_t intraperiod;                                       /**< Frame intraperiod. */
        uint32_t quantisationParameter;                             /**< Frame quantisation. */
        int bInlineHeaders;                                         /**< Inline header output. */
        int immutableInput;                                         /**< Flag indicating input buffer data will not change during encoding. */
        MMAL_VIDEO_PROFILE_T profile;                               /**< H264 video profile. */
        MMAL_VIDEO_LEVEL_T level;                                   /**< H264 video encoder level.*/
        MMAL_VIDEO_INTRA_REFRESH_T intra_refresh_type;              /**< H264 video intra refresh. */
        int inlineMotionVectors;                                    /**< Send inline motion vector data to callbacks. Default is true. */
    };

    /**
     \class RaspiEncoder RaspiEncoder.h "components/RaspiEncoder.h"
     \brief An H264 encoder component
     */
    class RaspiEncoder : public RaspiComponent {
        public:
            /**
             \brief Creates default encoder options.
             \return A RASPIENCODER_OPTION_S struct
             \see RASPIENCODER_OPTION_S
             */
            static RASPIENCODER_OPTION_S createDefaultEncoderOptions();

            /**
             \brief Creates an encoder component with supplied options.
             \return A shared pointer to an encoder component
             \see RaspiEncoder::createDefaultEncoderOptions()
             */
            static shared_ptr< RaspiEncoder > create(RASPIENCODER_OPTION_S options);

            /**
             \brief Creates an encoder component with default options.
             \return A shared pointer to an encoder component
             */
            static shared_ptr< RaspiEncoder > create();
            shared_ptr< RaspiPort > input;                  /**< The encoder's input port. This is the component's default_input. \see RaspiComponent#default_input */
            shared_ptr< RaspiPort > output;                 /**< The encoder's output port. This is the component's default_output. \see RaspiComponent#default_output */
        protected:
            const int MAX_BITRATE_MJPEG = 25000000;
            const int MAX_BITRATE_LEVEL4 = 25000000;
            const int MAX_BITRATE_LEVEL42 = 62500000;
            const char* component_name();
            MMAL_STATUS_T init();
            RASPIENCODER_OPTION_S options_;
    };
}

#endif /* __RASPIENCODER_H__ */
