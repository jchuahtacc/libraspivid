/**
 \file RaspiCallback.h
 */

#ifndef __RASPICALLBACK_H__
#define __RASPICALLBACK_H__

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"


namespace raspivid {
    /**
     \class RaspiCallback "RaspiCallback.h"
     \brief A wrapper class for implementing port callbacks
     */
    class RaspiCallback {
        public:
            /**
             \brief Callback function for buffer data. The buffer is automatically locked, released and returned by RaspiPort.
             \param port A C pointer to the MMAL_PORT_T where the callback is originating from.
             \param buffer A C pointer to the MMAL_BUFFER_HEADER_T containing buffer data.
             \see RaspiPort::add_callback
             */
            virtual void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) =0;
            
            /**
             \brief A callback function, called after the main callback function. This allows implementing classes to perform operations after the
             buffer has been released back to the port. This prevents the port from being starved of buffers due to long post processing operations.
             \see RaspiPort::add_callback
             \see RaspiCallback::callback
             */
            virtual void post_process() { };
    };
}


#endif /* __RASPICALLBACK_H__ */
