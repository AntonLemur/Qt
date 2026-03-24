#ifndef VLCMANAGER_H
#define VLCMANAGER_H

#include <vlc/vlc.h>
#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths>
#include <QtConcurrent>

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

        (void)QtConcurrent::run([this](){
            cleanOldThumbnails(30);
        });
    }

    ~VlcManager() {
        if (m_vlc) libvlc_release(m_vlc);
    }

    // очистка кэша
    void cleanOldThumbnails(int daysLimit = 7) {
        // Получаем путь к папке с кэшем миниатюр
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbs/";
        QDir dir(cacheDir);

        if (!dir.exists()) return;

        // Получаем текущую дату и время
        QDateTime now = QDateTime::currentDateTime();

        // Перебираем все файлы в папке
        QFileInfoList files = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : std::as_const(files)/*чтоб не копировал*/) {
            // Вычисляем возраст файла в днях
            qint64 ageInDays = fileInfo.lastModified().daysTo(now);

            if (ageInDays > daysLimit) {
                QFile::remove(fileInfo.absoluteFilePath());
                // qDebug() << "Удалена старая миниатюра:" << fileInfo.fileName();
            }
        }
    }

    libvlc_instance_t *m_vlc = nullptr;
};

#endif // VLCMANAGER_H
