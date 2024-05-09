#include "localdocsmodel.h"

#include "localdocs.h"
#include "network.h"

LocalDocsCollectionsModel::LocalDocsCollectionsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(LocalDocs::globalInstance()->localDocsModel());
}

bool LocalDocsCollectionsModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString collection = sourceModel()->data(index, LocalDocsModel::CollectionRole).toString();
    return m_collections.contains(collection);
}

void LocalDocsCollectionsModel::setCollections(const QList<QString> &collections)
{
    m_collections = collections;
    invalidateFilter();
}

LocalDocsModel::LocalDocsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LocalDocsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_collectionList.size();
}

QVariant LocalDocsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_collectionList.size())
        return QVariant();

    const CollectionItem item = m_collectionList.at(index.row());
    switch (role) {
        case CollectionRole:
            return item.collection;
        case FolderPathRole:
            return item.folder_path;
        case InstalledRole:
            return item.installed;
        case IndexingRole:
            return item.indexing;
        case ErrorRole:
            return item.error;
        case ForceIndexingRole:
            return item.forceIndexing;
        case CurrentDocsToIndexRole:
            return item.currentDocsToIndex;
        case TotalDocsToIndexRole:
            return item.totalDocsToIndex;
        case CurrentBytesToIndexRole:
            return quint64(item.currentBytesToIndex);
        case TotalBytesToIndexRole:
            return quint64(item.totalBytesToIndex);
        case CurrentEmbeddingsToIndexRole:
            return quint64(item.currentEmbeddingsToIndex);
        case TotalEmbeddingsToIndexRole:
            return quint64(item.totalEmbeddingsToIndex);
        case TotalDocsRole:
            return quint64(item.totalDocs);
        case TotalWordsRole:
            return quint64(item.totalWords);
        case TotalTokensRole:
            return quint64(item.totalTokens);
        case LastUpdateRole:
            return item.lastUpdate;
        case FileCurrentlyProcessingRole:
            return item.fileCurrentlyProcessing;
        case EmbeddingModelRole:
            return item.embeddingModel;
    }

    return QVariant();
}

QHash<int, QByteArray> LocalDocsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CollectionRole] = "collection";
    roles[FolderPathRole] = "folder_path";
    roles[InstalledRole] = "installed";
    roles[IndexingRole] = "indexing";
    roles[ErrorRole] = "error";
    roles[ForceIndexingRole] = "forceIndexing";
    roles[CurrentDocsToIndexRole] = "currentDocsToIndex";
    roles[TotalDocsToIndexRole] = "totalDocsToIndex";
    roles[CurrentBytesToIndexRole] = "currentBytesToIndex";
    roles[TotalBytesToIndexRole] = "totalBytesToIndex";
    roles[CurrentEmbeddingsToIndexRole] = "currentEmbeddingsToIndex";
    roles[TotalEmbeddingsToIndexRole] = "totalEmbeddingsToIndex";
    roles[TotalDocsRole] = "totalDocs";
    roles[TotalWordsRole] = "totalWords";
    roles[TotalTokensRole] = "totalTokens";
    roles[LastUpdateRole] = "lastUpdate";
    roles[FileCurrentlyProcessingRole] = "fileCurrentlyProcessing";
    roles[EmbeddingModelRole] = "embeddingModel";
    return roles;
}

void LocalDocsModel::updateCollectionItem(const CollectionItem &item)
{
    for (int i = 0; i < m_collectionList.size(); ++i) {
        CollectionItem &stored = m_collectionList[i];
        if (stored.folder_id != item.folder_id)
            continue;

        QVector<int> changed;
        if (stored.collection != item.collection)
            changed.append(CollectionRole);
        if (stored.folder_path != item.folder_path)
            changed.append(FolderPathRole);
        if (stored.installed != item.installed)
            changed.append(InstalledRole);
        if (stored.indexing != item.indexing)
            changed.append(IndexingRole);
        if (stored.error != item.error)
            changed.append(ErrorRole);
        if (stored.forceIndexing != item.forceIndexing)
            changed.append(ForceIndexingRole);
        if (stored.currentDocsToIndex != item.currentDocsToIndex)
            changed.append(CurrentDocsToIndexRole);
        if (stored.totalDocsToIndex != item.totalDocsToIndex)
            changed.append(TotalDocsToIndexRole);
        if (stored.currentBytesToIndex != item.currentBytesToIndex)
            changed.append(CurrentBytesToIndexRole);
        if (stored.totalBytesToIndex != item.totalBytesToIndex)
            changed.append(TotalBytesToIndexRole);
        if (stored.currentEmbeddingsToIndex != item.currentEmbeddingsToIndex)
            changed.append(CurrentEmbeddingsToIndexRole);
        if (stored.totalEmbeddingsToIndex != item.totalEmbeddingsToIndex)
            changed.append(TotalEmbeddingsToIndexRole);
        if (stored.totalDocs != item.totalDocs)
            changed.append(TotalDocsRole);
        if (stored.totalWords != item.totalWords)
            changed.append(TotalWordsRole);
        if (stored.totalTokens != item.totalTokens)
            changed.append(TotalTokensRole);
        if (stored.lastUpdate != item.lastUpdate)
            changed.append(LastUpdateRole);
        if (stored.fileCurrentlyProcessing != item.fileCurrentlyProcessing)
            changed.append(FileCurrentlyProcessingRole);
        if (stored.embeddingModel != item.embeddingModel)
            changed.append(EmbeddingModelRole);

        stored = item;
        emit dataChanged(this->index(i), this->index(i), changed);
    }
}

void LocalDocsModel::addCollectionItem(const CollectionItem &item)
{
    beginInsertRows(QModelIndex(), m_collectionList.size(), m_collectionList.size());
    m_collectionList.append(item);
    endInsertRows();
}

void LocalDocsModel::removeCollectionIf(std::function<bool(CollectionItem)> const &predicate) {
    for (int i = 0; i < m_collectionList.size();) {
        if (predicate(m_collectionList.at(i))) {
            beginRemoveRows(QModelIndex(), i, i);
            m_collectionList.removeAt(i);
            endRemoveRows();

            Network::globalInstance()->trackEvent("doc_collection_remove", {
                {"collection_count", m_collectionList.count()},
            });
        } else {
            ++i;
        }
    }
}

void LocalDocsModel::removeFolderById(int folder_id)
{
    removeCollectionIf([folder_id](const auto &c) { return c.folder_id == folder_id; });
}

void LocalDocsModel::removeCollectionPath(const QString &name, const QString &path)
{
    removeCollectionIf([&name, &path](const auto &c) { return c.collection == name && c.folder_path == path; });
}

void LocalDocsModel::collectionListUpdated(const QList<CollectionItem> &collectionList)
{
    beginResetModel();
    m_collectionList = collectionList;
    endResetModel();
}
