//
// Created by cpasjuste on 30/03/23.
//

#include "platform.h"
#include "bitmap.h"

using namespace p2d;

Bitmap::Bitmap(Surface *surface, const Utility::Vec2i &pos) : Widget() {
    m_surface = surface;
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(): %ix%i, bytes: %lu\r\n",
           m_surface->getSize().x, m_surface->getSize().y, m_surface->getPixelsSize());
}

Bitmap::Bitmap(const Io::File &file, const Utility::Vec2i &pos) {
    m_surface = new Surface(file);
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(bmp): %ix%i, bytes: %lu\r\n",
           m_surface->getSize().x, m_surface->getSize().y, m_surface->getPixelsSize());
}

void Bitmap::onDraw(const Utility::Vec2i &pos, bool draw) {
    if (!draw) return;

    Platform::instance()->getDisplay()->drawSurface((Surface *) m_surface, pos);

    // draw child's
    Widget::onDraw(pos, draw);
}

Bitmap::~Bitmap() {
    delete (m_surface);
}
