#pragma once

#include"clr_utils.h"
#include"string_utils.h"

namespace HyperCrown {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MultiSubFileImport
	/// </summary>
	public ref class MultiSubFileImport : public System::Windows::Forms::Form
	{
	private:
		std::vector<std::string> *targetVector;
		System::String^ newFileFilter;
	public:
		MultiSubFileImport(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			targetVector = NULL;
		}

		MultiSubFileImport(std::vector<std::string> *target, std::vector<std::string> targetNames, std::vector<std::string> targetTypes, System::String^ name, System::String^ filter)
		{
			InitializeComponent();
			targetVector = target;
			newFileFilter = filter;
			if (targetVector != NULL)
			{
				std::stringstream ss;
				for (unsigned int i = 0; i < target->size(); i++)
				{
					ListViewItem ^itm = gcnew ListViewItem();
					ListViewItem::ListViewSubItem ^sItm = gcnew ListViewItem::ListViewSubItem();
					std::string iStr;
					ss.clear();
					ss.str("");
					ss << i;
					ss >> iStr;
					if (i < targetNames.size())
					{
						itm->Text = std2system(targetNames[i]);
					}
					else
					{
						itm->Text = name + L"-" + std2system(iStr);
					}
					if (i < targetTypes.size())
					{
						sItm->Text = std2system(targetTypes[i]);
					}
					else
					{
						sItm->Text = L"????";
					}
					itm->SubItems->Add(sItm);
					subFileContentsListView->Items->Add(itm);
				}
			}
			this->Name = L"Sub-File Importer - " + name;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MultiSubFileImport()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListView^  subFileContentsListView;
	protected:
	private: System::Windows::Forms::ColumnHeader^  subFileNameHeader;
	private: System::Windows::Forms::ColumnHeader^  subFileTypeHeader;
	private: System::Windows::Forms::Button^  acceptButton;
	private: System::Windows::Forms::Button^  cancelButton;
	private: System::Windows::Forms::ContextMenuStrip^  subFileContextMenuStrip;
	private: System::Windows::Forms::ToolStripMenuItem^  importToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  addToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeToolStripMenuItem;
	private: System::ComponentModel::IContainer^  components;


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->subFileContentsListView = (gcnew System::Windows::Forms::ListView());
			this->subFileNameHeader = (gcnew System::Windows::Forms::ColumnHeader());
			this->subFileTypeHeader = (gcnew System::Windows::Forms::ColumnHeader());
			this->subFileContextMenuStrip = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->importToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->addToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->acceptButton = (gcnew System::Windows::Forms::Button());
			this->cancelButton = (gcnew System::Windows::Forms::Button());
			this->subFileContextMenuStrip->SuspendLayout();
			this->SuspendLayout();
			// 
			// subFileContentsListView
			// 
			this->subFileContentsListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->subFileContentsListView->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
				this->subFileNameHeader,
					this->subFileTypeHeader
			});
			this->subFileContentsListView->ContextMenuStrip = this->subFileContextMenuStrip;
			this->subFileContentsListView->FullRowSelect = true;
			this->subFileContentsListView->HideSelection = false;
			this->subFileContentsListView->Location = System::Drawing::Point(12, 12);
			this->subFileContentsListView->Name = L"subFileContentsListView";
			this->subFileContentsListView->Size = System::Drawing::Size(272, 252);
			this->subFileContentsListView->TabIndex = 0;
			this->subFileContentsListView->UseCompatibleStateImageBehavior = false;
			this->subFileContentsListView->View = System::Windows::Forms::View::Details;
			// 
			// subFileNameHeader
			// 
			this->subFileNameHeader->Text = L"Sub-Filename";
			this->subFileNameHeader->Width = 223;
			// 
			// subFileTypeHeader
			// 
			this->subFileTypeHeader->Text = L"Type";
			this->subFileTypeHeader->Width = 45;
			// 
			// subFileContextMenuStrip
			// 
			this->subFileContextMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->importToolStripMenuItem,
					this->addToolStripMenuItem, this->removeToolStripMenuItem
			});
			this->subFileContextMenuStrip->Name = L"subFileContextMenuStrip";
			this->subFileContextMenuStrip->Size = System::Drawing::Size(118, 70);
			// 
			// importToolStripMenuItem
			// 
			this->importToolStripMenuItem->Name = L"importToolStripMenuItem";
			this->importToolStripMenuItem->Size = System::Drawing::Size(117, 22);
			this->importToolStripMenuItem->Text = L"Import";
			this->importToolStripMenuItem->Click += gcnew System::EventHandler(this, &MultiSubFileImport::importToolStripMenuItem_Click);
			// 
			// addToolStripMenuItem
			// 
			this->addToolStripMenuItem->Name = L"addToolStripMenuItem";
			this->addToolStripMenuItem->Size = System::Drawing::Size(117, 22);
			this->addToolStripMenuItem->Text = L"Add";
			this->addToolStripMenuItem->Click += gcnew System::EventHandler(this, &MultiSubFileImport::addToolStripMenuItem_Click);
			// 
			// removeToolStripMenuItem
			// 
			this->removeToolStripMenuItem->Name = L"removeToolStripMenuItem";
			this->removeToolStripMenuItem->Size = System::Drawing::Size(117, 22);
			this->removeToolStripMenuItem->Text = L"Remove";
			this->removeToolStripMenuItem->Click += gcnew System::EventHandler(this, &MultiSubFileImport::removeToolStripMenuItem_Click);
			// 
			// acceptButton
			// 
			this->acceptButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->acceptButton->Location = System::Drawing::Point(291, 13);
			this->acceptButton->Name = L"acceptButton";
			this->acceptButton->Size = System::Drawing::Size(75, 23);
			this->acceptButton->TabIndex = 1;
			this->acceptButton->Text = L"Accept";
			this->acceptButton->UseVisualStyleBackColor = true;
			this->acceptButton->Click += gcnew System::EventHandler(this, &MultiSubFileImport::acceptButton_Click);
			// 
			// cancelButton
			// 
			this->cancelButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->cancelButton->Location = System::Drawing::Point(290, 241);
			this->cancelButton->Name = L"cancelButton";
			this->cancelButton->Size = System::Drawing::Size(75, 23);
			this->cancelButton->TabIndex = 2;
			this->cancelButton->Text = L"Cancel";
			this->cancelButton->UseVisualStyleBackColor = true;
			this->cancelButton->Click += gcnew System::EventHandler(this, &MultiSubFileImport::cancelButton_Click);
			// 
			// MultiSubFileImport
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(371, 276);
			this->Controls->Add(this->cancelButton);
			this->Controls->Add(this->acceptButton);
			this->Controls->Add(this->subFileContentsListView);
			this->Name = L"MultiSubFileImport";
			this->Text = L"Sub-File Importer";
			this->subFileContextMenuStrip->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void acceptButton_Click(System::Object^  sender, System::EventArgs^  e) {
		this->DialogResult = Windows::Forms::DialogResult::OK;
		this->Close();
	}
	private: System::Void cancelButton_Click(System::Object^  sender, System::EventArgs^  e) {
		this->DialogResult = Windows::Forms::DialogResult::Cancel;
		this->Close();
	}
	private: System::Void importToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator^ enm = subFileContentsListView->Items->GetEnumerator();
		while (enm->MoveNext())
		{
			if (((ListViewItem^)enm->Current)->Selected)
			{
				OpenFileDialog fd;
				fd.Filter = newFileFilter;
				fd.FilterIndex = 0;
				fd.AddExtension = true;
				if (fd.ShowDialog() == Windows::Forms::DialogResult::OK)
				{
					std::string fileDat;
					if (binaryFileRead(system2std(fd.FileName), fileDat))
					{
						(*targetVector)[((ListViewItem^)enm->Current)->Index] = fileDat;
						MessageBox::Show(this, L"Import Successful", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1);
					}
					else
					{
						MessageBox::Show(this, L"Failed to read file from file system", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					}
				}
				break;
			}
		}
	}
	private: System::Void addToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog fd;
		fd.Filter = newFileFilter;
		fd.FilterIndex = 0;
		fd.AddExtension = true;
		if (fd.ShowDialog() == Windows::Forms::DialogResult::OK)
		{
			std::string fileDat;
			if (binaryFileRead(system2std(fd.FileName), fileDat))
			{
				ListViewItem^ itm = gcnew ListViewItem();
				ListViewItem::ListViewSubItem^ sItm = gcnew ListViewItem::ListViewSubItem();
				unsigned int nInd = fd.FileName->LastIndexOf(L"\\") + 1;
				targetVector->push_back(fileDat);
				itm->Text = fd.FileName->Substring(nInd, fd.FileName->LastIndexOf(L".") - nInd);
				sItm->Text = L"????";
				itm->SubItems->Add(sItm);
				subFileContentsListView->Items->Add(itm);
				MessageBox::Show(this, L"Import Successful", L"HyperCrown - Import Sub-File",
					MessageBoxButtons::OK, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1);
			}
			else
			{
				MessageBox::Show(this, L"Failed to read file from file system", L"HyperCrown - Import Sub-File",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
	}
	private: System::Void removeToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator^ enm = subFileContentsListView->Items->GetEnumerator();
		while (enm->MoveNext())
		{
			if (((ListViewItem^)enm->Current)->Selected)
			{
				targetVector->erase(targetVector->begin() + ((ListViewItem^)enm->Current)->Index);
				subFileContentsListView->Items->RemoveAt(((ListViewItem^)enm->Current)->Index);
			}
		}
	}
};
}
