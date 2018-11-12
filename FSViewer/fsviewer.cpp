#include "FSViewer.h"


#include "foldermodel.h"
#include "FileSystem/BootSector.h"
#include "FileSystem/DirectoryTree.h"
#include "FileSystem/VirtualReader.h"
#include "FileSystem/fat_fs.h"

#include "IO/constants.h"

using namespace FileSystem;

#include "fstreemodel.h"
#include "fstablemodel.h"

#include <QDir>

FSViewer::FSViewer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	RecoverDialog_ = new RecoverDialog(this, &RecoverUi_);
	RecoverUi_.setupUi(RecoverDialog_);

	auto file_ptr = IO::makeFilePtr(LR"(d:\vdisk\1Gb.vhd)");
	file_ptr->OpenRead();
	
	SectorReader sector_reader(new CSectorReader(file_ptr, 512));

	DirectoryEntry root_folder(new DirectoryNode("root:"));

	if (sector_reader->Open())
	{
		MasterBootRecord MBR;


		if (MBR.open(sector_reader))
		{
			if (MBR.count() > 0)
			{

				abstract_fs = FatFS(new FatFileSystem(sector_reader));
				if (abstract_fs->mount(MBR.getPartition(0)))
				{
					NodeEntry resultEntry;

					if (abstract_fs->getFirst(root_folder, resultEntry))
					{
						do
						{

						} while (abstract_fs->getNext(resultEntry));
					}	// end if

				}
			}

		}
	}
	else
		printf("Error open device...\r\n");

	DirectoryEntry pMainRoot(new DirectoryNode(""));
	pMainRoot->add_folder(root_folder);

	TreeIndex * treeRootIndex = new TreeIndex(pMainRoot);
	tree_model_ = new FSTreeModel(this, treeRootIndex);
	ui.treeView->setModel(tree_model_);

	TableIndex * tableRootIndex = new TableIndex(pMainRoot);
	table_model_ = new FSTableModel(this, tableRootIndex);
	ui.tableView->setModel(table_model_);
	ui.tableView->setRootIndex(table_model_->indexFromPath(L"\\root:"));

	connect(ui.tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(OnTableContextMenu(const QPoint &)));
	connect(table_model_, SIGNAL(updateTreeState(const QString &, Qt::CheckState)), this, SLOT(TableViewCheckedFolders(const QString &, Qt::CheckState)));
	connect(table_model_, SIGNAL(updateParentState(Qt::CheckState)), this, SLOT(TableViewParentFolders(Qt::CheckState)));

	connect(tree_model_, SIGNAL(updateTableState(Qt::CheckState)), this, SLOT(TreeViewChecked(Qt::CheckState)));
	recFileAction = new QAction(tr("&Recover file"), this);
	ui.treeView->setBaseSize(100, 0);

}

FSViewer::~FSViewer()
{
	if (tree_model_)
	{
		delete tree_model_;
		tree_model_ = nullptr;
	}
	if (table_model_)
	{
		delete table_model_;
		table_model_ = nullptr;
	}
}


void FSViewer::RecoverFile(const QString & folder_path, const FileSystem::FileEntry & file_entry)
{
	QDir currentDir(folder_path);

	QFileInfo fileInfo(currentDir, QString::fromStdWString(file_entry->name()));


	if (!currentDir.mkpath(folder_path))
	{
		qDebug("Error to create directory...");
		return;
	}

	if ((file_entry->size() == 0) || (file_entry->size() == 4096))
	{
		DWORD bytesRead = 0;
		file_entry->OpenFile();
		//readSizeUsingTable
		auto fat_fs = std::dynamic_pointer_cast<FatFileSystem>(abstract_fs);
		if (fat_fs)
		{
			auto file_size = fat_fs->readSizeUsingTable(file_entry);
			if (file_size > 0)
			{
				BYTE * read_data = new BYTE[file_size];

				if (abstract_fs->ReadFile(file_entry, read_data, file_entry->size(), bytesRead))
				{
					qDebug("Read ok.");
				}
				QString filePath(fileInfo.absoluteFilePath());
				IO::File target_file(fileInfo.absoluteFilePath().toStdWString());
				target_file.OpenCreate();
				target_file.WriteData(read_data, bytesRead);
				delete read_data;
			}
		}
	}
/*
	QDir currentDir(folder_path);

	QFileInfo fileInfo(currentDir, QString::fromStdWString(file_entry->name()));


	if (!currentDir.mkpath(folder_path))
	{
		qDebug("Error to create directory...");
		return;
	}

	DWORD sector_count = FileSystem::sectorsFromSize(file_entry->size(), 512);

	BYTE * read_data = new BYTE[sector_count * 512];
	memset(read_data, 0x8F, sector_count * 512);
	DWORD bytesRead = 0;
	file_entry->OpenFile();
	if (abstract_fs->ReadFile(file_entry, read_data, file_entry->size(), bytesRead))
	{
		qDebug("Read ok.");
	}
	QString filePath(fileInfo.absoluteFilePath());
	HANDLE hWriteFile = CreateFile(fileInfo.absoluteFilePath().toStdWString().c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		0, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE)
	{
		qDebug("Error open file to write");
	}
	else
	{
		DWORD bytes_write = 0;
		BOOL bWriteResult = WriteFile(hWriteFile, read_data, file_entry->size(), &bytes_write, NULL);



		CloseHandle(hWriteFile);
	}
	delete read_data;
	*/
}
void FSViewer::RecoverFolder(const QString & folder_path, FileSystem::DirectoryEntry & folder_entry)
{
	QDir currentDir(folder_path);
	QChar delimiter('\\');
	QString next_folder(folder_path);
	if (next_folder.at(next_folder.length() - 1) != delimiter)
		next_folder.append(delimiter);
	next_folder.append(QString::fromStdWString(folder_entry->name()));


	if (!currentDir.mkpath(next_folder))
	{
		qDebug("Error to create directory...");
		return;
	}

	NodeEntry next_entry;
	if (abstract_fs->getFirst(folder_entry, next_entry))
	{
		do
		{
			if (FileSystem::DirectoryEntry dir_entry = std::dynamic_pointer_cast<DirectoryNode> (next_entry))
			{
				this->RecoverFolder(next_folder, dir_entry);
			}
			else
				if (FileSystem::FileEntry file_entry = std::dynamic_pointer_cast<FileNode> (next_entry))
					this->RecoverFile(next_folder, file_entry);

		} while (abstract_fs->getNext(next_entry));
	}
}
void FSViewer::RecoverFolder(const QString & folder_path, TableIndex * FolderIndex)
{
	QDir currentDir(folder_path);
	QChar delimiter('\\');
	QString next_folder(folder_path);
	if (next_folder.at(next_folder.length() - 1) != delimiter)
		next_folder.append(delimiter);
	next_folder.append(QString::fromStdWString(FolderIndex->getEntry()->name()));

	if (!currentDir.mkpath(next_folder))
	{
		qDebug("Error to create directory...");
		return;
	}

	//NodeEntry next_entry;
	readFolders(FolderIndex);

	for (std::size_t iNodeEntry = 0; iNodeEntry < FolderIndex->count(); ++iNodeEntry)
	{
		if (TableIndex * pChildIndex = FolderIndex->getChild(iNodeEntry))
		{
			if (pChildIndex->getEntry()->type() == FileSystem::folder_type)
			{
				this->RecoverFolder(next_folder, pChildIndex);
			}
			else
				if (FileSystem::FileEntry file_entry = std::dynamic_pointer_cast<FileNode> (pChildIndex->getEntry()))
					this->RecoverFile(next_folder, file_entry);
		}
	}
}

void FSViewer::OnTreeViewClicked(QModelIndex modelIndex)
{
	if (modelIndex.isValid())
	{
		TreeIndex * current_index = static_cast <TreeIndex *>
			(modelIndex.internalPointer());


		//IVirtualNode * pVirtualNode = static_cast < IVirtualNode* >
		//	( modelIndex.internalPointer() );
		//if (pVirtualNode)
		if (NodeEntry node_entry = current_index->getEntry())
		{
			wstring root_path(current_index->getRootPath());

			if (node_entry->count() == 0)
				if (node_entry->type() == folder_type)
				{
					NodeEntry childNodes;
					if (abstract_fs->getFirst(std::static_pointer_cast<DirectoryNode>(node_entry), childNodes))
					{
						do
						{

						} while (abstract_fs->getNext(childNodes));

						current_index->resetChilds();
						tree_model_->update_view(modelIndex);
						ui.treeView->setExpanded(modelIndex, ui.treeView->isExpanded(modelIndex));

						if (TableIndex * table_index = table_model_->indexByPath(root_path))
							table_index->resetChilds();

					}
				}
			QModelIndex root_index(table_model_->indexFromPath(root_path));
			ui.tableView->setRootIndex(root_index);
			ui.tableView->resizeColumnsToContents();
			ui.tableView->setColumnWidth(0, 350);
			ui.tableView->resizeRowsToContents();
		}


		//table_model_->setRootPath( current_index->getRootPath() );

		//ui.tableView->resizeColumnsToContents();
		//ui.tableView->setColumnWidth(0,350);
		//ui.tableView->resizeRowsToContents();
	}


}
void FSViewer::DoubleClickedTableView(QModelIndex modelIndex)
{
	if (modelIndex.isValid())
	{
		if (TableIndex * current_index = static_cast<TableIndex *> (modelIndex.internalPointer()))
		{
			wstring rootPath = current_index->getRootPath();
			QModelIndex treeIndex = tree_model_->indexFromPath(rootPath);
			if (treeIndex.isValid())
			{
				OnTreeViewClicked(treeIndex);
				ui.treeView->setCurrentIndex(treeIndex);
			}
		}
	}
}
void FSViewer::OnTableContextMenu(const QPoint & point_pos)
{
	QModelIndex cell = ui.tableView->indexAt(point_pos);
	if (cell.isValid())
	{
		//QString myid=cell.sibling(cell.row(),0).data().toString();
		TableIndex *selected_index = static_cast<TableIndex*> (cell.internalPointer());
		if (selected_index)
		{
			//pContexMenu->addAction("Recover selected", this, SLOT(Recovery(selected_index->getEntry())));
			if (selected_index->getEntry())
			{
				QMenu contextMenu(this);
				contextMenu.addAction(tr("Recover"), this, SLOT(RecoverySelected()));
				contextMenu.exec(ui.tableView->mapToGlobal(point_pos));
			}
		}
	}
}
void FSViewer::TableViewCheckedFolders(const QString & node_path, Qt::CheckState checkState)
{
	QModelIndex currentIndex = tree_model_->indexFromPath(node_path.toStdWString());
	if (currentIndex.isValid())
	{
		tree_model_->updateCheckState(currentIndex, checkState);
		tree_model_->update_view(currentIndex);
	}
}
void FSViewer::TableViewParentFolders(Qt::CheckState checkState)
{
	QModelIndex currentIndex = ui.treeView->currentIndex();

	while (currentIndex.isValid())
	{
		if (TableIndex * tableIndex = toTableIndex(ui.tableView->currentIndex()))
			if (TableIndex * parentIndex = tableIndex->parent())
			{
				checkState = parentIndex->checked();
				tree_model_->updateCheckState(currentIndex, checkState);
				tree_model_->update_view(currentIndex);
			}
		currentIndex = currentIndex.parent();
	}
}
void FSViewer::TreeViewChecked(Qt::CheckState checkState)
{
	//QItemSelectionModel *selModel = ui.treeView->selectionModel();
	//

	//if (selModel->currentIndex().isValid())
	//{
	//	if ( TreeIndex * pTreeIndex = toTreeIndex( selModel->currentIndex() ) )
	//	{
	//		read_and_check(pTreeIndex,checkState);
	//		pTreeIndex->updateCheckChilds(checkState);
	//		if (TableIndex * pTableIndex = table_model_->indexByPath(pTreeIndex->getRootPath()))
	//		{
	//			pTableIndex->updateCheckChilds(checkState);
	//			pTableIndex->updateCheckParents(checkState);
	//		}
	//		
	//		//tree_model_->update_view(currentIndex);
	//		/*ui.treeView->reset();*/
	//	}
	//}
}

void FSViewer::RecoverySelected()
{
	//QString folder_path(QDir::currentPath()); 
	QString folder_path("D:\\Recovery\\");
	RecoverDialog_->setFolderPath(folder_path);
	if (RecoverDialog_->exec() == QDialog::Accepted)
	{
		folder_path = RecoverDialog_->getFolderPath();

		QItemSelectionModel *selModel = ui.tableView->selectionModel();
		TableIndex *selected_index = static_cast<TableIndex*> (selModel->currentIndex().internalPointer());

		//if (FileSystem::DirectoryEntry rec_folder = std::tr1::dynamic_pointer_cast <DirectoryNode> ( selected_index->getEntry()) )
		//{
		//	qDebug("recovering folder...");
		//	RecoverFolder(folder_path,rec_folder);
		//}
		//else
		if (selected_index->getEntry()->type() == FileSystem::folder_type)
			RecoverFolder(folder_path, selected_index);
		if (FileSystem::FileEntry rec_file = std::dynamic_pointer_cast <FileNode> (selected_index->getEntry()))
		{
			qDebug("recovering file...");
			RecoverFile(folder_path, rec_file);
		}
	}
}

void FSViewer::readFolders(TableIndex * FolderIndex)
{
	NodeEntry childNodes;
	if (abstract_fs->getFirst(std::static_pointer_cast<DirectoryNode>(FolderIndex->getEntry()), childNodes))
	{
		do
		{

		} while (abstract_fs->getNext(childNodes));

		FolderIndex->resetChilds();

		if (TreeIndex * tree_index = tree_model_->indexByPath(FolderIndex->getRootPath()))
		{
			tree_index->resetChilds();
			QModelIndex treeIndex = tree_model_->indexFromPath(FolderIndex->getRootPath());
			if (treeIndex.isValid())
				tree_model_->update_view(treeIndex);
		}

	}
}
void FSViewer::read_and_check(TreeIndex * checkIndex, const Qt::CheckState checkState)
{
	if (NodeEntry node_entry = checkIndex->getEntry())
	{
		wstring root_path(checkIndex->getRootPath());

		if (node_entry->count() == 0)
		{
			if (node_entry->type() == folder_type)
			{
				NodeEntry childNodes;
				if (abstract_fs->getFirst(std::static_pointer_cast<DirectoryNode>(node_entry), childNodes))
				{
					do
					{

					} while (abstract_fs->getNext(childNodes));

					//checkIndex->setChecked(checkState);
					checkIndex->resetChilds();

					if (TableIndex * table_index = table_model_->indexByPath(root_path))
						table_index->resetChilds();

				}
			}
		}
	}
}

TreeIndex * FSViewer::toTreeIndex(const QModelIndex & treeIndex)
{
	return static_cast<TreeIndex*> (treeIndex.internalPointer());
}
TableIndex * FSViewer::toTableIndex(const QModelIndex & tableIndex)
{
	return static_cast<TableIndex*> (tableIndex.internalPointer());
}

