/**
 \brief RaspiCamera.h
 */

#ifndef __RASPICAMERA_H__
#define __RASPICAMERA_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "components/RaspiRenderer.h"
#include "RaspiPort.h"
#include "RaspiCamControl.h"

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3


namespace raspivid {

    using namespace raspi_cam_control;

    /**
     \class RaspiCameraCallback RaspiCamera.h "components/RaspiCamera.h"
     \brief An abstract class for callbacks for camera control port changes
     */
    class RaspiCameraCallback {
        public:
            virtual void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) =0;
    };

    /**
     \brief A struct for passing callbacks to the camera's control port
     */
    typedef struct {
        shared_ptr< RaspiCameraCallback > cb_instance;
    } RASPICAMERA_USERDATA_S;

    /**
     \class DefaultRaspiCameraCallback RaspiCamera.h "components/RaspiCamera.h"
     \brief Default camera control callback. Prints a message that a control parameter has changed.
     */
    class DefaultRaspiCameraCallback : public RaspiCameraCallback {
        public:
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };

    /**
     \brief A structure containing camera parameters. 
     */
    struct RASPICAMERA_OPTION_S {
        uint32_t height;                                        /**< Camera frame height */
        uint32_t width;                                         /**< Camera frame width */
        uint32_t framerate;                                     /**< Desired camera framerate */
        int cameraNum;                                          /**< Camera number. Usually 0. */
        int sensor_mode;                                        /**< Camera sensor mode. */
        bool verbose;                                           /**< Verbose debugging output */
        RASPICAM_CAMERA_PARAMETERS camera_parameters;           /**< RaspiCam parameter structure. \see RaspiCamControl.h */
        shared_ptr< RaspiCameraCallback > settings_callback;    /**< A shared pointer to a camera settings control callback */
    };

    /**
     \brief A component representing a Raspberry Pi Camera. There is no default_input port and the default_output port is the video port.
     \see RaspiComponent#default_output
     */
    class RaspiCamera : public RaspiComponent {
        public:
            shared_ptr< RaspiPort > still;                      /**< The camera's still photo port */
            shared_ptr< RaspiPort > video;                      /**< The camera's video port. This is the default_output port */
            shared_ptr< RaspiPort > preview;                    /**< The camera's preview video port. */

            /**
             \brief Creates RaspiCamera object. Uses the default settings.
             \return A shared ponter to a RaspiCamera object.
             \see RaspiCamera::createDefaultCameraOptions
             \see RaspiCamera::create(RASPICAMERA_OPTION_S options);
             */
            static shared_ptr< RaspiCamera > create();

            /**
             \brief Creates a RaspiCamera object with user supplied settings.
             \return A shared pointer to a RaspiCamera object.
             \see RaspiCamera::create()
             */
            static shared_ptr< RaspiCamera > create(RASPICAMERA_OPTION_S options);

            /**
             \brief Creates a struct with default camera settings.
             \return A struct containing default camera options.
             \see RASPICAMERA_OPTION_S
             */
            static RASPICAMERA_OPTION_S createDefaultCameraOptions();
            
            /**
             \brief Starts frame capture on this camera. Effectively, this turns the camera "on" and starts frame output on each port.
             \return An MMAL_STATUS_T. MMAL_SUCCESS if the operation was successful.
             */
            MMAL_STATUS_T start();
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
            RASPICAMERA_OPTION_S options_;
        private:
            static void callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            RASPICAMERA_USERDATA_S userdata;
    };
};

#endif /* __RASPICAMERA_H__ */
