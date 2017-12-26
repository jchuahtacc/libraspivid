#include "components/RaspiOverlayRenderer.h"

namespace raspivid {
    
    RASPIOVERLAYRENDERER_FORMAT_S RaspiOverlayRenderer::createDefaultOverlayFormat() {
        RASPIOVERLAYRENDERER_FORMAT_S result;
        RASPIPORT_FORMAT_S format = RaspiPort::createDefaultPortFormat();
        result.encoding = MMAL_ENCODING_RGB24;
        result.width = format.width;
        result.height = format.height;
        result.layer = 128;
        result.alpha = 255;
        result.crop = format.crop;
        result.dest.x = 0;
        result.dest.y = 0;
        result.dest.width = format.width;
        result.dest.height = format.height;
        result.fullscreen = true;
        return result;
    }

    MMAL_BUFFER_HEADER_T* RaspiOverlayRenderer::get_buffer() {
        return input->get_buffer();
    }

    void RaspiOverlayRenderer::send_buffer(MMAL_BUFFER_HEADER_T *buffer) {
        input->send_buffer(buffer);
    }

    shared_ptr< RaspiOverlayRenderer > RaspiOverlayRenderer::create() {
        return RaspiOverlayRenderer::create(RaspiOverlayRenderer::createDefaultOverlayFormat());  
    }

    void RaspiOverlayRenderer::callback_(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        mmal_buffer_header_release(buffer);
    }

    shared_ptr< RaspiOverlayRenderer > RaspiOverlayRenderer::create(RASPIOVERLAYRENDERER_FORMAT_S format) {
        shared_ptr< RaspiOverlayRenderer > result = shared_ptr< RaspiOverlayRenderer > (new RaspiOverlayRenderer());
        result->format_ = format;
        if (result->init() != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;   
    }

    MMAL_STATUS_T RaspiOverlayRenderer::init() {

        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        MMAL_PORT_T *input_port = component->input[0];
        input_port->format->encoding = format_.encoding;
        input_port->format->es->video.width = VCOS_ALIGN_UP(format_.width, 32);
        input_port->format->es->video.height = VCOS_ALIGN_UP(format_.height, 16);
        input_port->format->es->video.crop = format_.crop;
        
        if ((status = mmal_port_format_commit(input_port)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiOverlayRenderer::init(): could not set port format");
            return status;
        }

        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiOverlayRenderer::init(): could not enable component");
            return status;
        }

        input_port->buffer_size = input_port->buffer_size_recommended;
        input_port->buffer_num = input_port->buffer_num_recommended;
        if (input_port->buffer_num < 2) {
            input_port->buffer_num = 2;
        }

        input = RaspiPort::create(input_port, "RaspiOverlayRenderer::input");
        default_input = input;
        
        if ((status = input->create_buffer_pool()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiOverlayrenderer::init(): could not create port buffer pool");
            return status;
        }

        {
            MMAL_DISPLAYREGION_T param;
            param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
            param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

            param.set = MMAL_DISPLAY_SET_LAYER;
            param.layer = format_.layer;

            param.set |= MMAL_DISPLAY_SET_ALPHA;
            param.alpha = format_.alpha;

            param.set |= (MMAL_DISPLAY_SET_DEST_RECT | MMAL_DISPLAY_SET_FULLSCREEN);
            param.fullscreen = format_.fullscreen;
            param.dest_rect = format_.dest;
            if ((status = mmal_port_parameter_set(input_port, &param.hdr)) != MMAL_SUCCESS) {
                vcos_log_error("RaspiOverlayRenderer::init(): Couldn't set port display region");
                return status;
            }
        }

        if ((status = mmal_port_enable(input_port, callback_)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiOverlayRenderer::init(): Couldn't enable input port callback");
            return status;
        }

        return MMAL_SUCCESS;
    }

}
