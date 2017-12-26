#include "components/RaspiComponent.h"

namespace raspivid {

    MMAL_STATUS_T RaspiComponent::init() {
        MMAL_STATUS_T status;
        
        if ((status = mmal_component_create(component_name(), &component)) != MMAL_SUCCESS) {
            vcos_log_error("RaspiComponent::init(): unable to create component %s", component_name());
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiComponent::connect( shared_ptr< RaspiComponent > source_component ) {
        if ( default_input && source_component->default_output ) {
            return connect( source_component->default_output );
        } else {
            return MMAL_EINVAL;
        }
    }

    MMAL_STATUS_T RaspiComponent::connect( shared_ptr< RaspiPort > source_port ) {
        if ( default_input && source_port ) {
            return default_input->connect( source_port );
        } else {
            return MMAL_EINVAL;
        }
    }

    void RaspiComponent::assert_ports(int inputs, int outputs) {
        vcos_assert(component);
        vcos_assert(component->input_num >= inputs);
        vcos_assert(component->output_num >= outputs);
    }

    void RaspiComponent::destroy() {
        if (component) {
            mmal_component_disable(component);
            mmal_component_destroy(component);
            component = NULL;
        }
    }

    RaspiComponent::RaspiComponent() {
    }

    RaspiComponent::~RaspiComponent() {
        destroy();
    }

}
