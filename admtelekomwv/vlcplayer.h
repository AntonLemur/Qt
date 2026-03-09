#ifndef VLCPLAYER_H
#define VLCPLAYER_H

#include <vlc/vlc.h>
#include "vlc/libvlc_media.h"
#include "vlc/libvlc_media_player.h"
#include <QtQml/qqmlregistration.h> //для QML_ELEMENT
#include <QThread>
#include <QMutex>
#include <vector>
#include <QPointer>
#include <QVideoSink>
#include <qqml.h>
#include "frameworker.h"

// using namespace QNativeInterface;

// extern "C" {
// // We declare the function we found using nm
// void libvlc_media_player_set_android_context(libvlc_media_player_t *p_mi, void *p_jvm_or_surface);
// }

class VLCPlayer : public QObject {
    Q_OBJECT
    QML_ELEMENT // This makes the class available for use/instantiation on the QML side.
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(bool playbackState READ playbackState NOTIFY playbackStateChanged)
    // Q_PROPERTY(float positionbylength READ positionbylength NOTIFY positionbylengthChanged)
    Q_PROPERTY(u_int64_t position READ position NOTIFY positionChanged) //в мс
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(u_int64_t duration READ duration NOTIFY durationChanged)
public:
    VLCPlayer();

   // 1. LOCK: VLC asks where to write the pixels of the next frame
   static void* lock(void *opaque, void **planes);

   // 2. UNLOCK: The frame has been written to the buffer
   static void unlock(void *opaque, void *picture, void *const *planes);

   // 3. DISPLAY: It's time to tell Qt that the image has been updated
   static void display(void *opaque, void *picture);

   // 4. Ловим события VLC
   static void vlc_callback(const libvlc_event_t *event, void *data);

   QVideoSink *videoSink() const;
   void setVideoSink(QVideoSink *newVideoSink);

   Q_INVOKABLE void setSource(const QString &path);
   Q_INVOKABLE void play();
   Q_INVOKABLE void stop();
   Q_INVOKABLE void pause();
   Q_INVOKABLE void setPosition(u_int64_t ms);
   bool playbackState() const;
   // float positionbylength() const;
   u_int64_t position() const;
   bool seekable() const;
   u_int64_t duration() const;

    // Video settings (can be changed)
    const int VIDEO_WIDTH = 1280;
    const int VIDEO_HEIGHT = 720;

    std::vector<uchar> videoBuffer;
    QMutex bufferMutex;
    libvlc_media_player_t *mp = nullptr;
signals:
    void videoSinkChanged();
    void playbackStateChanged();
    // void positionbylength();
    void positionChanged();
    void seekableChanged();
    void durationChanged();

private:
    libvlc_instance_t *_vlcInstance;
    libvlc_media_player_t *_vlcPlayer;
    QPointer<QVideoSink> m_videoSink;
    bool m_bplaybackState = false;
    QVideoFrame m_currentFrame;
    QVideoFrameFormat m_format; // Инициализируйте один раз (VIDEO_WIDTH, VIDEO_HEIGHT, RGBA8888)
};

#endif // VLCPLAYER_H
