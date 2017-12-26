/**
 \file RaspiPort.h
 */


#ifndef __RASPIPORT_H__
#define __RASPIPORT_H__

#include <memory>
#include <string>
#include "RaspiCallback.h"

using namespace std;

namespace raspivid {
    /**
     \typedef RASPIPORT_USERDATA_S;
     \brief An internal structure that manages callback data.

     \see RaspiPort::add_callback
      */
    typedef struct {
        shared_ptr< RaspiCallback > cb_instance;
        MMAL_POOL_T* pool;
    } RASPIPORT_USERDATA_S;

    /**
     \typedef RASPIPORT_FORMAT_S
     \brief A structure that represents common port settings
      */
    typedef struct {
        uint32_t encoding;              /**< Frame encoding format */
        uint32_t encoding_variant;      /**< Frame encoding variant, if encoding format is MMAL_ENCODING_OPAQUE */
        uint32_t width;                 /**< Port width */
        uint32_t height;                /**< Port height */
        MMAL_RECT_T crop;               /**< Cropping rectangle */
        uint32_t frame_rate_num;        /**< Desired frame rate numerator */
        uint32_t frame_rate_den;        /**< Desired frame rate denominator */
    } RASPIPORT_FORMAT_S;

    /**
     \class RaspiPort "RaspiPort.h"
     \brief A wrapper class to manage a component port.

        Ports are initialized by components and manage/destroy their own connections. 
        By default, all ports will have a default resolution of 1920x1080 with 
        a base encoding of MMAL_ENCODING_OPAQUE with the MMAL_ENCODING_I420 variant (which is YUV planar data.)
        In addition, MMAL_ZERO_COPY is set for each port.

     */
    class RaspiPort {
        public:
            /**
             \brief Returns a struct containing default port settings
             \return a RASPIPORT_FORMAT_S
             \see RASPIPORT_FORMAT_S
            */
            static RASPIPORT_FORMAT_S createDefaultPortFormat();

            /**
             \brief Returns a shared pointer to a new RaspiPort.

             RaspiComponent child classes in this library will call this upon initialization.
             \return a shared_ptr representing a new RaspiPort
             */
            static shared_ptr< RaspiPort > create(MMAL_PORT_T *port, string port_name_ = "port_name");

            /**
             \brief Sets the port format.
             \param new_format A RASPIPORT_FORMAT_S struct.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             \see RaspiPort::get_format
             \see RASPIPORT_FORMAT_S
             */
            MMAL_STATUS_T set_format(RASPIPORT_FORMAT_S new_format);

            /**
             \brief Gets the current port format.
             \return A RASPIPORT_FORMAT_S representing the current port format.
             \see RASPIPORT_FORMAT_S
             \see RaspiPort::set_format
             */
            RASPIPORT_FORMAT_S get_format();

            /**
             \brief Adds a callback to this port.
             \param callback A shared pointer to a RaspiCallback instance.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             \see RaspiCallback
             */
            MMAL_STATUS_T add_callback(shared_ptr< RaspiCallback > callback);

            /**
             \brief Connects this port to another port.
             \param output The port providing output frames to this port.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             */
            MMAL_STATUS_T connect(shared_ptr< RaspiPort > output);

            /**
             \brief Connects this port to an underlying MMAL_PORT_T.
             \param output[in] A C pointer to an underlying MMAL_PORT_T providing frames to this port.
             \param connection[out] A C pointer to an MMAL_CONNECTION_T C pointer where conncetion information will be stored.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             */
            MMAL_STATUS_T connect(MMAL_PORT_T *output, MMAL_CONNECTION_T **connection);

            /**
             \brief Allocates a buffer pool for this port. Useful if a client program must manually supply frames to this port.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             \see RaspiPort::get_buffer
             \see RaspiPort::send_buffer
             */
            MMAL_STATUS_T create_buffer_pool();

            /**
             \brief Gets an allocated buffer from this port's buffer pool. User supplied frames may be written to this buffer.
             \return A C pointer to an MMAL_BUFFER_HEADER_T
             \see RaspiPort::create_buffer_pool
             \see RaspiPort::send_buffer
             */
            MMAL_BUFFER_HEADER_T* get_buffer();

            /**
             \brief Sends a buffer to this port. Buffers containing user supplied frames may be sent to this port.
             \param buffer[in] A C pointer to an MMAL_BUFFER_HEADER_T.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             */
            MMAL_STATUS_T send_buffer(MMAL_BUFFER_HEADER_T *buffer);
            
            /**
             \brief Turns on "zero copy" optimization. This optimization allows sharing of MMAL_ENCODING_I420 frames between the GPU and 
             ARM client memory space. Automatically called upon RaspiPort creation.
             \return An MMAL_STATUS_T (MMAL_SUCCESS if the operation was successful).
             */
            MMAL_STATUS_T set_zero_copy();

            /**
             \brief Cleans up the port, connection and any callbacks or buffers on this port
             */

            /**
             \brief Class destructor
             */
            void destroy();
            ~RaspiPort();

            string port_name; /**< Port name. Default value is "port_name" */
        protected:
            RaspiPort(MMAL_PORT_T *port, string port_name_);
        private:
            static void callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            RASPIPORT_USERDATA_S userdata;
            MMAL_POOL_T *pool;
            MMAL_PORT_T *port;
            MMAL_CONNECTION_T *connection;
    };

}


#endif /* __RASPIPORT_H__ */
