//
// Created by cpasjuste on 30/03/23.
//

#include "platform.h"
#include "bitmap.h"

using namespace p2d;

Bitmap::Bitmap(Surface *surface, const Utility::Vec2i &pos) : Widget() {
    m_surface = surface;
    m_read_only = true;
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(): %ix%i\r\n", m_surface->getSize().x, m_surface->getSize().y);
}

Bitmap::Bitmap(const Io::File &file, const Utility::Vec2i &pos) {
    m_surface = new Surface(file);
    Widget::setPosition(pos);
    Widget::setSize(m_surface->getSize());
    printf("Bitmap(): %s, %ix%i, %lubytes\r\n", file.getPath().c_str(),
           m_surface->getSize().x, m_surface->getSize().y, m_surface->getPixelsSize());
}

void Bitmap::onDraw(bool draw) {
    if (!draw) return;

    const Utility::Vec2i p = {m_bounds.x, m_bounds.y};
    Platform::instance()->getDisplay()->drawSurface(m_surface, p);

    // draw child's
    Widget::onDraw(draw);
}

Bitmap::~Bitmap() {
    if (!m_read_only) delete (m_surface);
}
