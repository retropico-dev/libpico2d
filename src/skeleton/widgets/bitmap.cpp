//
// Created by cpasjuste on 30/03/23.
//

#include "platform.h"
#include "bitmap.h"

using namespace mb;

Bitmap::Bitmap(Surface *surface, const Utility::Vec2i &pos) : Widget() {
    m_surface = surface;
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(): %ix%i, bytes: %lu\r\n",
           m_surface->getSize().x, m_surface->getSize().y, m_surface->getPixelsSize());
}

Bitmap::Bitmap(const uint8_t *bmp, const Utility::Vec2i &pos) {
    m_surface = new Surface(bmp);
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(bmp): %ix%i, bytes: %lu\r\n",
           m_surface->getSize().x, m_surface->getSize().y, m_surface->getPixelsSize());
}

void Bitmap::loop(const Utility::Vec2i &pos, const uint16_t &buttons) {
    if (!isVisible()) return;

    Platform::instance()->getDisplay()->drawSurface(m_surface, pos);

    // draw child's
    Widget::loop(pos, buttons);
}
