/**
 \file RaspiSplitter.h
 */
#ifndef __RASPISPLITTER_H__
#define __RASPISPLITTER_H__

#include <memory>
#include "raspivid/components/RaspiComponent.h"
#include "raspivid/RaspiPort.h"


//#define VIDEO_FRAME_RATE_NUM 30
//#define VIDEO_FRAME_RATE_DEN 1

namespace raspivid {
      
    /**
     \class RaspiSplitter RaspiSplitter.h "components/RaspiSplitter.h"
     \brief A splitter that duplexes port input.
     */
    class RaspiSplitter : public RaspiComponent {
        public:
            /**
             \brief Creates a splitter component.
             \return A shared pointer to a RaspiSplitter.
             */
            static shared_ptr< RaspiSplitter > create();
            shared_ptr< RaspiPort > input;      /**< The input port. This is the default_input for this component. \see RaspiComponent::default_input */
            shared_ptr< RaspiPort > output_0;   /**< An output port. This is the default_output for this component. \see RaspiComponent::default_output */
            shared_ptr< RaspiPort > output_1;   /**< A secondary duplicated output. */
            MMAL_STATUS_T connect( shared_ptr< RaspiComponent > component ); /**< \see RaspiComponent::connect( shared_ptr< RaspiComponent > component ) */
            MMAL_STATUS_T connect( shared_ptr< RaspiPort > src ); /**< \see RaspiComponent::connect( shared_ptr< RaspiPort > src */
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* __RASPISPLITTER_H__ */
