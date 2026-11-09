#pragma once

#include <QHash>
#include <QList>
#include <QObject>

#include "downloadpriority.h"

namespace BitTorrent
{
    class Torrent;

    class LargeTorrentSequentialManager final : public QObject
    {
        Q_OBJECT
    public:
        explicit LargeTorrentSequentialManager(QObject *parent = nullptr);
        void process(Torrent *torrent);
        void remove(Torrent *torrent);
        void clear();

    private:
        struct State
        {
            QList<DownloadPriority> original;
            QList<DownloadPriority> expected;
        };
        QHash<Torrent *, State> m_states;
    };
}
