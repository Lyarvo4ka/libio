#ifndef FSVIEWER_H
#define FSVIEWER_H

#include <QtGui/QMainWindow>
#include "ui_fsviewer.h"

class FSTreeModel;
class FSTableModel;


class FolderModel;
class FS_Model;
class TableIndex;
class TreeIndex;
#include "AbstractFS.h"

//	-- declaration
//class IVirtualNode;
//typedef std::tr1::shared_ptr<IVirtualNode> NodeEntry;
//class FileNode;
//typedef std::tr1::shared_ptr<FileNode> FileEntry;
//class DirectoryNode;
//typedef std::tr1::shared_ptr<DirectoryNode> DirectoryEntry;

#include "ui_RecoverDialog.h"
#include "recoverdialog.h"

class FSViewer : public QMainWindow
{
	Q_OBJECT

public:
	typedef std::tr1::shared_ptr<QDialog> DialogPtr;

	FSViewer(QWidget *parent = 0, Qt::WFlags flags = 0);
	~FSViewer();

	void RecoverFile(const QString & folder_path, const FileSystem::FileEntry & file_entry);
	void RecoverFolder(const QString & folder_path, FileSystem::DirectoryEntry & folder_entry);
	void RecoverFolder(const QString & folder_path, TableIndex * FolderIndex);


signals:
	void recoverFileSignal(const FileSystem::NodeEntry & node_entry);
protected slots:
	void OnTreeViewClicked(QModelIndex modelIndex);
	void DoubleClickedTableView(QModelIndex modelIndex);
	void OnTableContextMenu(const QPoint & point_pos);

	void TableViewCheckedFolders(const QString & node_path, Qt::CheckState checkState);
	void TableViewParentFolders(Qt::CheckState checkState);
	void TreeViewChecked(Qt::CheckState checkState);

	void RecoverySelected();
private:
	void readFolders( TableIndex * FolderIndex );
	void read_and_check( TreeIndex * checkIndex, const Qt::CheckState checkState);
private:
	TreeIndex * toTreeIndex(const QModelIndex & treeIndex);
	TableIndex * toTableIndex(const QModelIndex & tableIndex);

	Ui::FSViewerClass ui;
	FSTreeModel * tree_model_;
	FSTableModel * table_model_;

	//	Dialogs
	Ui::RecoverDialog RecoverUi_;
	RecoverDialog * RecoverDialog_;
	// Actions
	QAction *recFileAction;

	FolderModel * pFSmodel;
	FS_Model * pFullMode;

	FileSystem::AbstractFS abstract_fs;
};

#endif // FSVIEWER_H
