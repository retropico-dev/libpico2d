//
// Created by cpasjuste on 30/05/23.
//

/**
 * Copyright (C) 2023 by cpasjuste <cpasjuste@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "platform.h"
#include "text.h"
#include "tweeny.h"
#include "bitmap.h"

#define SPRITES_COUNT 10

using namespace p2d;

static Display::Settings ds{
    .displaySize = {240, 240}, // real display size (hardware size)
    .renderSize = {240, 240}, // rendering size (framebuffer size)
    .renderBounds = {0, 0, 240, 240}, // rendering bounds, used for scaling (if renderSize != renderBounds.w/h)
    .bufferingMode = Display::Buffering::Double,
    .format = Display::Format::RGB565
};

int main() {
    const auto platform = new P2DPlatform(ds);
    platform->getDisplay()->setClearColor(Display::Color::Black);

    const auto size = platform->getDisplay()->getSize();
    const auto center = Utility::Vec2i((int16_t) (size.x / 2), (int16_t) (size.y / 2));

    // load bitmap resources
    const auto surface = new Surface(Io::File{"res:/romfs/girl_120x120.bmp"});
    const auto girl = new Bitmap(surface);
    platform->add(girl);
    for (int i = 1; i < SPRITES_COUNT + 1; i++) {
        const auto g = new Bitmap(surface);
        g->setPosition(static_cast<int16_t>(i * 8), static_cast<int16_t>(i * 8));
        platform->add(g);
    }

    const auto star = new Bitmap(Io::File{"res:/romfs/star.bmp"}, center);
    star->setOrigin(Widget::Origin::Center);
    platform->add(star);

    const auto text = new Text((int16_t) (size.x - 2), (int16_t) (size.y - 2), "Hello MicroBoy");
    text->setColor(Display::Color::Red);
    text->setOrigin(Widget::Origin::BottomRight);
    platform->add(text);

    /*
    Clock tweenClock;
    auto tween = tweeny::from(girl->getPosition().x)
            .to(platform->getDisplay()->getSize().x)
            .during(5 * 1000);
    */

    while (platform->loop()) {
        //int16_t x = tween.step((int32_t) tweenClock.restart().asMilliseconds(), true);
        //girl->setPosition(x, girl->getPosition().y);
    }

    // delete girl surface
    delete surface;

    // reboot to bootloader
    platform->reboot();

    //...
    delete (platform);

    return 0;
}
