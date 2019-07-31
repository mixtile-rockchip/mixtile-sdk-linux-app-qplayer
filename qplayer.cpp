/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplayer.h"
#include <QtWidgets>
#include <QVideoSurfaceFormat>

void VideoWin::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        if(vp->isUIon())
            vp->hideUI();
        else
            vp->showUI();
    }
}

QPlayer::QPlayer(QWidget *parent)
    : QWidget(parent)
    , player(0, QMediaPlayer::VideoSurface)
    , list(0)
    , playButton(0)
    , positionSlider(0)
{
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);

    exitButton = new QPushButton(tr("Exit"));
    connect(exitButton, &QAbstractButton::clicked, this, &QPlayer::exit);

    playButton = new QPushButton;
//    playButton->setEnabled(false);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(playButton, &QAbstractButton::clicked, this, &QPlayer::play);

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 0);
    connect(positionSlider, &QAbstractSlider::sliderMoved, this, &QPlayer::setPosition);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(exitButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);

    QBoxLayout *layout = new QVBoxLayout(this);
    videoWidget = new VideoWin;
    videoWidget->setVideoPlayer(this);
    layout->setMargin(0);
    layout->addWidget(videoWidget);
    layout->addLayout(controlLayout);

    list = new QMediaPlaylist;
    list->setPlaybackMode(QMediaPlaylist::Loop);
    player.setVideoOutput(videoWidget);
    connect(&player, &QMediaPlayer::stateChanged, this, &QPlayer::mediaStateChanged);
    connect(&player, &QMediaPlayer::positionChanged, this, &QPlayer::positionChanged);
    connect(&player, &QMediaPlayer::durationChanged, this, &QPlayer::durationChanged);
    connect(&player, &QMediaPlayer::mediaStatusChanged, this, &QPlayer::mediaStatusChanged);
    connect(&timer, &QTimer::timeout, this, &QPlayer::next);
    showUI();
}

QPlayer::~QPlayer()
{
}

void QPlayer::hideUI()
{
    playButton->hide();
    exitButton->hide();
    positionSlider->hide();
    uiOn = false;
}

void QPlayer::showUI()
{
    playButton->show();
    exitButton->show();
    positionSlider->show();
    uiOn = true;
}

bool QPlayer::isPlayerAvailable() const
{
    return player.isAvailable();
}

void QPlayer::exit()
{
    qApp->exit(0);
}

void QPlayer::next()
{
    timer.stop();
    list->setCurrentIndex(list->nextIndex());
    play();
}

void QPlayer::setPlaylist(QStringList l)
{
    if(! l.empty()){
        for(int i = 0; i < l.size(); i++){
            QString str = l.at(i);
            QFile f(l.at(i));
            QUrl u(str);

            if(f.exists()){
                list->addMedia(QUrl::fromLocalFile(str));
            }else if(u.isValid()){
                list->addMedia(u);
            }

        }
        list->setCurrentIndex(1);
        player.setPlaylist(list);
    }
}

void QPlayer::load(const QUrl &url)
{
    player.setMedia(url);
    playButton->setEnabled(true);
}

void QPlayer::play()
{
    switch(player.state()) {
    case QMediaPlayer::PlayingState:
        player.pause();
        break;
    default:
        player.play();
        break;
    }
}

void QPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void QPlayer::positionChanged(qint64 position)
{
    positionSlider->setValue(position);
}

void QPlayer::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
}

void QPlayer::setPosition(int position)
{
    player.setPosition(position);
}

void QPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::EndOfMedia){
        QMediaContent media = list->currentMedia();
        QUrl url = media.canonicalUrl();
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForUrl(url);
        QString s = mt.name().mid(0, 6);
        if(! s.compare("image/")){
            player.pause();
            timer.start(3000);
        }
    }
}
