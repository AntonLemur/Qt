#ifndef VLCMANAGER_H
#define VLCMANAGER_H

#include <vlc/vlc.h>
#include <QObject>

class VlcManager {
public:
    static libvlc_instance_t* instance() {
        static VlcManager manager;
        return manager.m_vlc;
    }

private:
    VlcManager() {
        const char *vlc_args[] = {
            // "--no-audio",
            // "--no-video-title-show",
            // "--avcodec-hw=none",
            // "--network-caching=1500", // Уменьшаем время ожидания для плохих ссылок
            // "--rtsp-tcp",
            // "--no-stats"

            "--no-drop-late-frames", // Разрешить VLC выбрасывать кадры, которые не успели вовремя
            "--no-skip-frames",
            "--rtsp-frame-buffer-size=100000" // Увеличить размер системного буфера для UDP пакетов
        };
        m_vlc = libvlc_new(0, nullptr); //libvlc_new(3, vlc_args);
    }

    ~VlcManager() {
        if (m_vlc) libvlc_release(m_vlc);
    }

    libvlc_instance_t *m_vlc = nullptr;
};

#endif // VLCMANAGER_H
