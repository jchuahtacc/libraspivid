/**
 \file RaspiRenderer.h
 */

#ifndef __RASPIRENDERER_H__
#define __RASPIRENDERER_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "RaspiPort.h"

/// Layer that preview window should be displayed on
#define PREVIEW_LAYER      2

// Frames rates of 0 implies variable, but denominator needs to be 1 to prevent div by 0
#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1

#define FULL_RES_PREVIEW_FRAME_RATE_NUM 0
#define FULL_RES_PREVIEW_FRAME_RATE_DEN 1

#define FULL_FOV_PREVIEW_16x9_X 1280
#define FULL_FOV_PREVIEW_16x9_Y 720

#define FULL_FOV_PREVIEW_4x3_X 1296
#define FULL_FOV_PREVIEW_4x3_Y 972

#define FULL_FOV_PREVIEW_FRAME_RATE_NUM 0
#define FULL_FOV_PREVIEW_FRAME_RATE_DEN 1

namespace raspivid {

    /**
     \class RaspiRenderer RaspiRenderer.h "components/RaspiRenderer.h"
     \brief Creates a simple preview renderer. Connect this to a RaspiCamera's preview port to get a video preview of camera output.
     \see RaspiCamera#preview
     */
    class RaspiRenderer : public RaspiComponent {
        public:
            /**
             \brief Creates a RaspiRenderer with the specified alpha transparency and layer.
             \param alpha The alpha transparency of the preview.
             \param layer The layer at which the preview is rendered.
             \return A shared pointer to a RaspiRenderer.
             */
            static shared_ptr< RaspiRenderer > create(int alpha, int layer);

            /**
             \brief Creates a RaspiRenderer with an alpha value of 255 (opaque) at layer 2.
             \return A shared pointer to a RaspiRenderer.
             */
            static shared_ptr< RaspiRenderer > create();
            shared_ptr< RaspiPort > input;      /**< The input port for this renderer. This is the default_input for this component. \see RaspiComponent::default_input */
        protected:
            const char* component_name();
            int alpha_ = 255;
            int layer_ = PREVIEW_LAYER;
            MMAL_STATUS_T init();
            MMAL_RECT_T rendererWindow;
    };
}

#endif /* RASPIRENDERER_H_ */
