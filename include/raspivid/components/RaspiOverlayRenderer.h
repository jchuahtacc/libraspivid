/**
 \file RaspiOverlayRenderer.h
 */

#ifndef __RASPIOVERLAYRENDERER_H__
#define __RASPIOVERLAYRENDERER_H__

#include <memory>
#include "raspivid/components/RaspiComponent.h"
#include "raspivid/RaspiPort.h"
#include "raspivid/components/RaspiRenderer.h"
#include "raspivid/RaspiCallback.h"

namespace raspivid {

    /**
     \brief Overlay renderer format option structure
     */
    typedef struct {
        uint32_t encoding;              /**< Desired encoding. The default format is RGB24 */
        uint32_t width;                 /**< Frame width */
        uint32_t height;                /**< Frame height */
        uint32_t layer;                 /**< Overlay layer. Default is 128 */
        uint32_t alpha;                 /**< Overlay alpha. Default is 200 */
        MMAL_BOOL_T fullscreen;         /**< Flag for whether the overlay should occupy the full screen. Default is true. */
        MMAL_RECT_T crop;               /**< Overlay cropping rectangle. */
        MMAL_RECT_T dest;               /**< Overlay screen destination rectangle. */
    } RASPIOVERLAYRENDERER_FORMAT_S;

    /**
     \class RaspiOverlayRenderer RaspiOverlayRenderer.h "components/RaspiOverlayRenderer.h"
     \brief A component used for drawing overlays. Client code that uses an overlay renderer should retrieve a buffer, write to the buffer, then send the buffer back to
     the overlay renderer.
     \see RaspiOverlayRenderer::get_buffer
     \see RaspiOverlayRenderer::send_buffer
     */
    class RaspiOverlayRenderer : public RaspiRenderer {
        public:
            /**
             \brief Gets a buffer where overlay data can be written to.
             \return A C pointer to an MMAL_BUFFER_HEADER_T where data can be written.
             \see RaspiOverlayRenderer::send_buffer
             */
            MMAL_BUFFER_HEADER_T* get_buffer();

            /**
             \brief Sends a buffer back to the overlay renderer to be drawn.
             \param buffer[in] A C pointer to an MMAL_BUFFER_HEADER_T to be drawn. This buffer should be retrieved from get_buffer.
             \see RaspiOverlayRenderer::get_buffer
             */
            void send_buffer(MMAL_BUFFER_HEADER_T *buffer);

            /**
             \brief Returns a RASPIOVERLAYRENDERER_FORMAT_S struct with default settings.
             \return A RASPIOVERLAYRENDER_FORMAT_S struct.
             \see RASPIOVERLAYRENDRER_FORMAT_S
             */
            static RASPIOVERLAYRENDERER_FORMAT_S createDefaultOverlayFormat();

            /**
             \brief Creates an overlay renderer with default settings.
             \return A shared pointer to a RaspiOverlayRenderer
             \see RaspiOverlayRenderer::createDefaultOverlayFormat
             */
            static shared_ptr< RaspiOverlayRenderer > create();

            /**
             \brief Creates an overlay renderer with supplied settings.
             \param format A RASPIVOERLAYRENDERER_FORMAT_S with user defined setings.
             \return A shared pointer to a RaspiOverlayRenderer
             */
            static shared_ptr< RaspiOverlayRenderer >create(RASPIOVERLAYRENDERER_FORMAT_S format);
        protected:
            MMAL_STATUS_T init();
            RASPIOVERLAYRENDERER_FORMAT_S format_;
            static void callback_(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };
}

#endif /* RASPIOVERLAYRENDERER_H_ */
