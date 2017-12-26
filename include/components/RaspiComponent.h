/**
 \file components/RaspiComponent.h
 */
#ifndef __RASPICOMPONENT_H__
#define __RASPICOMPONENT_H__ 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory.h>
#include "RaspiPort.h"

#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"

using namespace std;

namespace raspivid {

    /**
     \class RaspiComponent RaspiComponent.h "components/RaspiComponent.h"
     \brief An abstract base class for all Raspberry Pi MMAL components.
     */
    class RaspiComponent {
        public:
            /**
             \brief Class destructor.
             */
            ~RaspiComponent();

            /**
             \brief Shuts down and cleans up component.
             */
            void destroy();

            /**
             \brief Connects another component's default_output to this component's default_input.
             \param source_component The component with a default_output port supplying frames to this component's default_input port
             \return An MMAL_STATUS_T result. MMAL_SUCCESS_T if successful, MMAL_EINVAL if the supplied component has no default_output port or if this
             component has no default_input port
             \see #default_input
             \see #default_output
             */
            MMAL_STATUS_T connect(shared_ptr< RaspiComponent > source_component);

            /**
             \brief Connects RaspiPort to this component's default_input.
             \param source_port The port supplying frames to this component's default_input port
             \return An MMAL_STATUS_T result. MMAL_SUCCESS_T if successful, MMAL_EINVAL if this component has no default_input port.
             \see RaspiComponent::connect
             \see #default_input
             \see #default_output
             */
            MMAL_STATUS_T connect(shared_ptr< RaspiPort > source_port);
            shared_ptr< RaspiPort > default_input = nullptr;    /**< Default input port for this component */
            shared_ptr< RaspiPort > default_output = nullptr;   /**< Default output port for this component */
        protected:
            RaspiComponent();
            MMAL_STATUS_T init();
            MMAL_COMPONENT_T *component;
            virtual const char* component_name() =0;
            void assert_ports(int inputs, int outputs);
    };
}

#endif /* __RASPICOMPONENT_H__  */
