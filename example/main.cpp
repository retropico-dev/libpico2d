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
//#include "bitmaps/star.h"
//#include "bitmaps/monster.h"
//#include "bitmaps/explosion-sprite.h"
//#include "bitmaps/girl_240x240_alpha.h"
#include "bitmaps/girl_120x120.h"

extern const unsigned char girl_120x120_bmp[];
extern const unsigned girl_120x120_bmp_size;

using namespace mb;

int main() {
    Clock clock, deltaClock;
    int frames = 0;

    auto platform = new P2DPlatform(Display::Buffering::Double);
    platform->addDisplay((Display *) new P2DDisplay({240, 240}, {120, 120}));

    platform->getDisplay()->setTextSize(2);

    auto center = new Utility::Vec2i((int16_t) (platform->getSize().x / 2),
                                     (int16_t) (platform->getSize().y / 2));

    auto surface = Surface::fromBmp(girl_120x120_bmp);
    auto bitmap = new Bitmap({}, surface);
    platform->add(bitmap);

    //auto bitmap = new Bitmap({}, (Surface *) &girl_120x120_surface);
    //bitmap->setOrigin(Widget::Origin::Center);
    //platform->add(bitmap);

    /*
    auto sprite = new Bitmap({center->x, center->y}, (Surface *) &explosion_sprite_surf);
    sprite->setOrigin(Widget::Origin::Center);
    platform->add(sprite);

    auto bitmap = new Bitmap({center->x, center->y}, (Surface *) &girl_240x240_alpha_surface);
    bitmap->setOrigin(Widget::Origin::Center);
    platform->add(bitmap);

    for (int16_t i = 0; i < 64; i++) {
        auto b = new Bitmap({(int16_t)(i * 2), (int16_t)(i * 2)}, (Surface *) &star_surface);
        platform->add(b);
    }
    */

    /*
    auto text = new Text(center->x, center->y, "MiamMiam");
    text->setColor(Display::Color::Black);
    text->setOrigin(Widget::Origin::Center);
    platform->add(text);
    */

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
