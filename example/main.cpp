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

using namespace p2d;

int main() {
    Clock clock; //deltaClock;
    int frames = 0;

    auto platform = new P2DPlatform(false);
    platform->addDisplay((Display *) new P2DDisplay({240, 240}, {120, 120}));
    platform->getDisplay()->setClearColor(Display::Color::Blue);


    auto bounds = platform->getDisplay()->getBounds();
    auto center = Utility::Vec2i((int16_t) (bounds.w / 2), (int16_t) (bounds.h / 2));

    // load bitmap resources
    auto girl = new Bitmap(romfs::get("girl_120x120.bmp"));
    platform->add(girl);

    auto star = new Bitmap(romfs::get("star.bmp"), center);
    star->setOrigin(Widget::Origin::Center);
    platform->add(star);

    auto text = new Text((int16_t) (bounds.w - 2), (int16_t) (bounds.h - 2), "Hello MicroBoy");
    text->setColor(Display::Color::Red);
    text->setOrigin(Widget::Origin::BottomRight);
    platform->add(text);

    /*
    auto tween = tweeny::from(bitmap->getPosition().x)
            .to(platform->getDisplay()->getSize().x)
            .during(5 * 1000);

    deltaClock.restart();
    */

    while (platform->loop(true)) {
        //int16_t x = tween.step((int32_t) deltaClock.restart().asMilliseconds(), true);
        //bitmap->setPosition(x, bitmap->getPosition().y);

        // fps
        if (clock.getElapsedTime().asSeconds() >= 1) {
            auto percent = (uint16_t) (((float) Utility::getUsedHeap() / (float) Utility::getTotalHeap()) * 100);
            printf("fps: %i, heap: %i/%i (%i%%)\r\n",
                   (int) ((float) frames / clock.restart().asSeconds()),
                   Utility::getUsedHeap(), Utility::getTotalHeap(), percent);
            frames = 0;
        }

        // increment frames for fps counter
        frames++;
    }

    // reboot to bootloader
    platform->reboot();
    delete (platform);

    return 0;
}