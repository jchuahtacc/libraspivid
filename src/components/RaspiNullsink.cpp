#include "components/RaspiNullsink.h"

namespace raspivid {
    const char* RaspiNullsink::component_name() {
        return "vc.null_sink";
    }

    shared_ptr< RaspiNullsink > RaspiNullsink::create() {
        shared_ptr< RaspiNullsink > result = shared_ptr< RaspiNullsink >( new RaspiNullsink() );
        if (result->init() != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;
    }

    MMAL_STATUS_T RaspiNullsink::init() {
        MMAL_STATUS_T status;

        if ((status = RaspiComponent::init()) != MMAL_SUCCESS) {
            return status;
        }

        assert_ports(1, 0);
        if ((status = mmal_component_enable(component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiNullsink::init(): unable to enable nullsink component (%u)", status);
            return status;
        }

        input = RaspiPort::create(component->input[0], "RaspiNullsink::input");
        default_input = input;

        return MMAL_SUCCESS;
    }

}
