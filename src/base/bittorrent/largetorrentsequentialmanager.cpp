#include "largetorrentsequentialmanager.h"

#include "base/preferences.h"
#include "torrent.h"

using namespace BitTorrent;

namespace
{
    // Start the next file early enough for external download monitors to see
    // forward progress before the current file is finalized at 100%.
    constexpr qreal FILE_HANDOFF_PROGRESS = 0.95;
}

LargeTorrentSequentialManager::LargeTorrentSequentialManager(QObject *parent)
    : QObject(parent) {}

void LargeTorrentSequentialManager::remove(Torrent *torrent)
{
    if (auto it = m_states.find(torrent); it != m_states.end())
    {
        torrent->prioritizeFiles(it->original);
        m_states.erase(it);
    }
}

void LargeTorrentSequentialManager::clear()
{
    const auto states = m_states.keys();
    for (Torrent *torrent : states)
        remove(torrent);
}

void LargeTorrentSequentialManager::process(Torrent *torrent)
{
    if (!torrent || !torrent->hasMetadata())
        return;
    const auto *pref = Preferences::instance();
    const bool eligible = pref->largeTorrentSequentialDownloadEnabled()
        && (torrent->totalSize() >= pref->largeTorrentSequentialThreshold());
    if (!eligible)
    {
        remove(torrent);
        return;
    }

    auto it = m_states.find(torrent);
    if (it == m_states.end())
    {
        State state;
        state.original = torrent->filePriorities();
        state.expected = state.original;
        it = m_states.insert(torrent, std::move(state));
    }

    const QList<DownloadPriority> actual = torrent->filePriorities();
    if (actual.size() == it->expected.size())
    {
        for (qsizetype i = 0; i < actual.size(); ++i)
        {
            if (actual.at(i) != it->expected.at(i))
                it->original[i] = actual.at(i);
        }
    }

    const QList<qreal> progress = torrent->filesProgress();
    QList<DownloadPriority> priorities = it->original;
    int current = -1;
    for (int i = 0; i < torrent->filesCount(); ++i)
    {
        if (torrent->fileSize(i) <= 0 || (i < progress.size() && progress.at(i) >= FILE_HANDOFF_PROGRESS))
            continue;
        if (it->original.at(i) == DownloadPriority::Ignored)
            continue;
        current = i;
        break;
    }
    if (current >= 0)
    {
        // Keep every file that has already started and is still incomplete at
        // maximum priority. The handoff intentionally starts the next file at
        // 95%, so earlier episodes can still need pieces to finish.
        for (int i = 0; i < priorities.size() && i < progress.size(); ++i)
        {
            if ((progress.at(i) > 0) && (progress.at(i) < 1)
                && (it->original.at(i) != DownloadPriority::Ignored))
                priorities[i] = DownloadPriority::Maximum;
        }
        priorities[current] = DownloadPriority::Maximum;
        for (int i = current + 1; i < priorities.size(); ++i)
            if (it->original.at(i) != DownloadPriority::Ignored)
                priorities[i] = DownloadPriority::Ignored;
        torrent->prioritizeFiles(priorities);
        it->expected = priorities;
    }
    else if (it->expected != it->original)
    {
        torrent->prioritizeFiles(it->original);
        it->expected = it->original;
    }
}
