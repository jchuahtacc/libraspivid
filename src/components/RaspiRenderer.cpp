#include "raspivid/components/RaspiRenderer.h"

namespace raspivid {
    const char* RaspiRenderer::component_name() {
        return MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER;
    }

    shared_ptr< RaspiRenderer > RaspiRenderer::create(int alpha, int layer) {
        shared_ptr< RaspiRenderer > result = shared_ptr< RaspiRenderer >( new RaspiRenderer() );
        result->alpha_ = alpha;
        result->layer_ = layer;
        if (result->init() != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;
    }

    shared_ptr< RaspiRenderer > RaspiRenderer::create() {
        return create(255, PREVIEW_LAYER);
    }

    /**
     * Create the preview component, set up its ports
     *
     * @param state Pointer to state control struct
     *
     * @return MMAL_SUCCESS if all OK, something else otherwise
     *
     */
    MMAL_STATUS_T RaspiRenderer::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 0);

        MMAL_PORT_T *mmal_input = component->input[0];
        input = RaspiPort::create(mmal_input, "RaspiRenderer::input");
        default_input = input;

        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = layer_;

        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = alpha_;

        param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
        param.fullscreen = 1;

        status = mmal_port_parameter_set(mmal_input, &param.hdr);

        if (status != MMAL_SUCCESS && status != MMAL_ENOSYS) {
             vcos_log_error("RaspiRenderer::init(): unable to set renderer port parameters (%u)", status);
             return status;
        }

        /* Enable component */
        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiRenderer::init(): unable to enable renderer component (%u)", status);
            return status;
        }

        return MMAL_SUCCESS;
    }

}
