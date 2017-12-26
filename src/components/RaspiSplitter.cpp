#include "components/RaspiSplitter.h"

namespace raspivid {
    const char* RaspiSplitter::component_name() {
        return MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER;
    }

    shared_ptr< RaspiSplitter > RaspiSplitter::create() {
        shared_ptr< RaspiSplitter > result = shared_ptr< RaspiSplitter >( new RaspiSplitter() );
        if (result->init() != MMAL_SUCCESS) {
            // delete result;
            return nullptr;
        }
        return result;
    }

    MMAL_STATUS_T RaspiSplitter::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 2);

        MMAL_PORT_T *mmal_input = component->input[0];
        MMAL_PORT_T *mmal_output_0 = component->output[0];
        MMAL_PORT_T *mmal_output_1 = component->output[1];

        input = RaspiPort::create(mmal_input, "RaspiSplitter::input");
        output_0 = RaspiPort::create(mmal_output_0, "RaspiSplitter::output_0");
        output_1 = RaspiPort::create(mmal_output_1, "RaspiSplitter::output_1");
        default_input = input;
        default_output = output_0;

        vcos_log_error("RaspiSplitter::init(): success!");

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiSplitter::connect( shared_ptr< RaspiComponent > source_component ) {
        return RaspiComponent::connect( source_component );
    }

    MMAL_STATUS_T RaspiSplitter::connect( shared_ptr< RaspiPort > source_port ) {
        MMAL_STATUS_T status;
        if ((status = RaspiComponent::connect( source_port )) != MMAL_SUCCESS) {
            return status;
        }
        RASPIPORT_FORMAT_S format = input->get_format();
        if ((status = output_0->set_format(format)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiSplitter::connect(): Unable to set output_0 format to input format");
            return status;
        }
        if ((status = output_1->set_format(format)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiSplitter::connect(): Unable to set output_1 format to input format");
            return status;
        }
        return MMAL_SUCCESS;
    }
}
