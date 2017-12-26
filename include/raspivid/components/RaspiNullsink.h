/**
 \file RaspiNullsink.h
 */

#ifndef __RASPINULLSINK_H__
#define __RASPINULLSINK_H__

#include <memory>
#include "raspivid/components/RaspiComponent.h"
#include "raspivid/RaspiPort.h"

namespace raspivid {

    /**
     \class RaspiNullsink RaspiNullsink.h "components/RaspiNullsink.h"
     \brief A nullsink component. Should be connected to unused ports where frames should be discarded, such as the camera still port when only video port data is desired."
     */
    class RaspiNullsink : public RaspiComponent {
        public:
            /**
             \brief Creates an instance of a nullsink component
             \return A shared pointer to a nullsink component
             */
            static shared_ptr< RaspiNullsink > create();
            shared_ptr< RaspiPort > input;          /**< The nullsink's input port. This is the default_input port. \see RaspiComponent#default_input */
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
    };
}

#endif /* RASPIRENDERER_H_ */
