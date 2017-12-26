#include "raspivid/RaspiPort.h"

namespace raspivid {

    RaspiPort::~RaspiPort() {
        destroy();
    }

    void RaspiPort::destroy() {
        if (connection) {
            mmal_connection_destroy(connection);
            connection = NULL;
        } else {
            if (port && port->is_enabled) {
                mmal_port_disable(port);
            }
            if (pool) {
                mmal_port_pool_destroy(port, pool);
                pool = NULL;
            }
        }
    }

    shared_ptr< RaspiPort > RaspiPort::create(MMAL_PORT_T *mmal_port, string port_name_) {
        return shared_ptr< RaspiPort >( new RaspiPort(mmal_port, port_name_ ) );
    }

    RaspiPort::RaspiPort(MMAL_PORT_T *mmal_port, string port_name_) : port(mmal_port), port_name(port_name_), pool(NULL) {
        set_zero_copy();
    }

    MMAL_STATUS_T RaspiPort::set_zero_copy() {
        MMAL_STATUS_T status;

        if ((status = mmal_port_parameter_set_boolean(port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::set_zero_copy: could not set zero copy on %s", port_name.c_str());
        }

        return status;
    }

    RASPIPORT_FORMAT_S RaspiPort::createDefaultPortFormat() {
        RASPIPORT_FORMAT_S result;
        result.encoding = MMAL_ENCODING_OPAQUE;
        result.encoding_variant = MMAL_ENCODING_I420;
        result.width = 1920;
        result.height = 1080;
        result.crop.x = 0;
        result.crop.y = 0;
        result.crop.width = 0;
        result.crop.height = 0;
        result.frame_rate_num = 0;
        result.frame_rate_den = 1;
        return result;
    }

    MMAL_STATUS_T RaspiPort::set_format(RASPIPORT_FORMAT_S options) {
        vcos_assert(port);
        MMAL_ES_FORMAT_T *format = port->format;
        format->encoding = options.encoding;
        format->encoding_variant = options.encoding_variant;
        format->es->video.width = VCOS_ALIGN_UP(options.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(options.height, 16);
        format->es->video.crop.x = options.crop.x;
        format->es->video.crop.y = options.crop.y;
        format->es->video.crop.width = options.crop.width ? options.crop.width : options.width;
        format->es->video.crop.height = options.crop.height ? options.crop.height : options.height;
        format->es->video.frame_rate.num = options.frame_rate_num;
        format->es->video.frame_rate.den = options.frame_rate_den;

        MMAL_STATUS_T status;
        if ((status = mmal_port_format_commit(port)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::format(): unable to commit port format");
            return status;
        }
        return MMAL_SUCCESS;
    }

    RASPIPORT_FORMAT_S RaspiPort::get_format() {
        vcos_assert(port);
        MMAL_ES_FORMAT_T *format = port->format;
        RASPIPORT_FORMAT_S result;
        result.encoding = format->encoding;
        result.encoding_variant = format->encoding_variant;
        result.width = format->es->video.width;
        result.height = format->es->video.height;
        result.crop = format->es->video.crop;
        result.frame_rate_num = format->es->video.frame_rate.num;
        result.frame_rate_den = format->es->video.frame_rate.den;
        return result;
    }


    MMAL_STATUS_T RaspiPort::connect(MMAL_PORT_T *output_port, MMAL_CONNECTION_T **connection) {
        vcos_assert(output_port);
        vcos_assert(port);
        MMAL_STATUS_T status;
        mmal_format_copy(port->format, output_port->format);
        if ((status = mmal_port_format_commit(port)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to commit new port format");
        }
        if ((status = mmal_connection_create(connection, output_port, port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to connect port");
            return status;
        }

        if ((status = mmal_connection_enable(*connection)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::connect(): unable to enable connection");
            mmal_connection_destroy(*connection);
            connection = NULL;
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiPort::connect(shared_ptr< RaspiPort > output_port) {
        vcos_assert(output_port);
        vcos_assert(output_port->port);
        return connect(output_port->port, &connection);
    }

    void RaspiPort::callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        RASPIPORT_USERDATA_S *userdata = (RASPIPORT_USERDATA_S *)port->userdata;
        vcos_assert(userdata);
        mmal_buffer_header_mem_lock(buffer);
        userdata->cb_instance->callback(port, buffer);
        mmal_buffer_header_mem_unlock(buffer);
        MMAL_POOL_T *pool = userdata->pool;
        mmal_buffer_header_release(buffer);
        if (pool && port->is_enabled) {
            MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(pool->queue);
            if (new_buffer) {
                if (mmal_port_send_buffer(port, new_buffer) != MMAL_SUCCESS) {
                    vcos_log_error("RaspiPort::callback_wrapper(): unable to return a buffer");
                }
            }
        }
        userdata->cb_instance->post_process();
    }

    MMAL_BUFFER_HEADER_T* RaspiPort::get_buffer() {
        vcos_assert(pool);
        return mmal_queue_wait(pool->queue);
    }

    MMAL_STATUS_T RaspiPort::send_buffer(MMAL_BUFFER_HEADER_T *buffer) {
        buffer->length = buffer->alloc_size;
        return mmal_port_send_buffer(port, buffer);
    }

    MMAL_STATUS_T RaspiPort::create_buffer_pool() {
        vcos_assert(port);
        if (!pool) {
            //port->buffer_num = port->buffer_num_recommended;
            //port->buffer_size = port->buffer_size_recommended;
            /*
            if (port->buffer_num < 3) {
                port->buffer_num = 3;
            }
            */
            vcos_log_error("RaspiPort::create_buffer_pool(): creating %d buffers of size %d for port %s", port->buffer_num, port->buffer_size, port_name.c_str());
            pool = mmal_port_pool_create(port, port->buffer_num, port->buffer_size);
            if (!pool) {
                vcos_log_error("RaspiPort::create_buffer_pool(): unable to create buffer pool");
                return MMAL_ENOSYS;
            }
        } else {
            vcos_log_error("RaspiPort::create_buffer_pool(): buffer pool already created for port");
        }
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiPort::add_callback(shared_ptr< RaspiCallback > cb_instance) {
        port->userdata = (struct MMAL_PORT_USERDATA_T *)&userdata;
        
        userdata.cb_instance = cb_instance;

        MMAL_STATUS_T status;

        if ((status = mmal_port_enable(port, callback_wrapper)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::add_callback(): unable to setup callback on port");
            return status;
        }

        
        if ((status = create_buffer_pool()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiPort::add_callback(): unable to allocate buffers for callback");
            return status;
        }
       
        userdata.pool = pool;
        int queue_length = mmal_queue_length(pool->queue);
        for (int i = 0; i < queue_length; i++) {
            MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
            if (!buffer) {
                vcos_log_error("RaspiPort:add_callback(): unable to get buffer from pool");
            }
            if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS) {
                vcos_log_error("RaspiPort::add_callback(): unable to send buffer to output port");
            }
        }


        return MMAL_SUCCESS;
    }
}
