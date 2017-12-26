/**
 \file RaspiResize.h
 */

#ifndef __RASPIRESIZE_H__
#define __RASPIRESIZE_H__

#include <memory>
#include "raspivid/components/RaspiComponent.h"
#include "raspivid/RaspiPort.h"


namespace raspivid {
      
    /**
     \class RaspiResize RaspiResize.h "components/RaspiResize.h"
     \brief A resizer component. The default output format is MMAL_ENCODING_I420 (YUV planar)
     */
    class RaspiResize : public RaspiComponent {
        public:
            /**
             \brief Creates a resizer component with the specified output dimensions
             \param width The output frame width
             \param height The output frame height
             \return A shared pointer to a RaspiResize component
             */
            static shared_ptr< RaspiResize > create(int width, int height);
            shared_ptr< RaspiPort > input;      /**< The input port for this component. This is the default_input port. \see RaspiComponent#default_input */
            shared_ptr< RaspiPort > output;     /**< The output port for this component. This is the default_output port. \see RaspiComponent#default_output */
            MMAL_STATUS_T connect( shared_ptr< RaspiComponent > source_component ); /**< \see RaspiComponent::connect( shared_ptr< RaspiComponent > source_component ) */
            MMAL_STATUS_T connect( shared_ptr< RaspiPort > source_port ); /**< \see RaspiComponent::connect( shared_ptr< RaspiPort > source_port */
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
            int width_;
            int height_;
    };
}

#endif /* __RASPIRESIZE_H__ */
