#pragma once

#include<limits>
#include<math.h>
#include<vector>
#include<string>
#include<sstream>

#include"clr_utils.h"
#include"mdls.h"
#include"MultiSubFileImport.h"
#include"PaletteAssignment.h"
#include"string_utils.h"

// Include Assimp Libraries
#include<assimp\Importer.hpp>
#include<assimp\postprocess.h>
#include<assimp\scene.h>
#include<assimp\types.h>

namespace HyperCrown {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Notify
	/// </summary>
	public ref class BaseForm : public System::Windows::Forms::Form
	{
	private: std::vector<JointRelative> *modelJoints;
	private: std::vector<std::vector<VertexRelative>> *modelVertsR, *shadowVertsR;
	private: std::vector<std::vector<VertexGlobal>> *modelVertsG, *shadowVertsG;
	private: std::vector<std::vector<Face>> *modelFaces, *shadowFaces;
	private: std::vector<Texture> *assetTextures, *msetBaseTextures, *msetTextures;
	private: std::vector<unsigned int> *refInds;
	private: std::vector<std::string> *assetSubfiles, *assetSubfileTypes;
	private: std::vector<bool> *assetSubfileStatus;
	private: std::string *assetData, *msetData, *templateData, *assetName, *msetName, *assetType;
	private: System::Windows::Forms::RichTextBox^  templateTextBox;
	private: System::Windows::Forms::Button^  msetButton;
	private: System::Windows::Forms::RichTextBox^  msetTextBox;
	private: System::Windows::Forms::CheckBox^  flipCheckBox;
	private: System::Windows::Forms::CheckBox^  yCheckBox;
	private: System::Windows::Forms::CheckBox^  normalCheckBox;
	private: System::Windows::Forms::CheckBox^  combineCheckBox;
	private: System::Windows::Forms::ImageList^  assetTextureList;
	private: System::Windows::Forms::ImageList^  msetTextureList;
	private: System::Windows::Forms::ListView^  assetTextureListView;
	private: System::Windows::Forms::ListView^  msetTextureListView;
	private: System::Windows::Forms::ColumnHeader^  texture;
	private: System::Windows::Forms::ContextMenuStrip^  assetTextureListViewContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  importAssetImageToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportAssetImagesToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportAllAssetImagesToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  addAssetImageToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeAssetImagesToolStripMenuItem;
	private: System::Windows::Forms::ContextMenuStrip^  msetTextureListViewContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  importMsetImageToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportMsetImagesToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportAllMsetImagesToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  addMsetImageToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeMsetImagesToolStripMenuItem;
	private: System::Windows::Forms::NumericUpDown^  msetNumericUpDown;
	private: System::Windows::Forms::ProgressBar^  exportProgressBar;
	private: System::Windows::Forms::ToolStripMenuItem^  assignPaletteToPixelsToolStripMenuItem;
	private: System::Windows::Forms::ListView^  assetSubFileListView;
	private: System::Windows::Forms::ColumnHeader^  assetSubFilename;
	private: System::Windows::Forms::ListView^  msetSubFileListView;
	private: System::Windows::Forms::ColumnHeader^  msetSubFilename;
	private: System::Windows::Forms::ContextMenuStrip^  assetSubFileListViewContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  importAssetSubFileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportAssetSubFilesToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportAssetAllToolStripMenuItem;
	private: System::Windows::Forms::ColumnHeader^  assetSubFileType;
	private: System::Windows::Forms::ColumnHeader^  assetSubFileExists;
	private: System::Windows::Forms::ColumnHeader^  msetSubFileType;
	private: System::Windows::Forms::ColumnHeader^  msetSubFileExists;
	private: System::Windows::Forms::ContextMenuStrip^  msetSubFileListViewContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  importMsetSubFileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportMsetSubFileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exportMsetAllToolStripMenuItem;
	private: System::Windows::Forms::ListViewItem^ focusedMsetItem;

	public:
		BaseForm(void)
		{
			Texture t;
			InitializeComponent();
			assetData = new std::string();
			msetData = new std::string();
			templateData = new std::string();
			assetName = new std::string();
			assetType = new std::string();
			assetTextures = new std::vector<Texture>();
			msetBaseTextures = new std::vector<Texture>();
			msetTextures = new std::vector<Texture>();
			msetName = new std::string();
			refInds = new std::vector<unsigned int>();
			modelJoints = new std::vector<JointRelative>();
			modelVertsR = new std::vector<std::vector<VertexRelative>>();
			shadowVertsR = new std::vector<std::vector<VertexRelative>>();
			modelVertsG = new std::vector<std::vector<VertexGlobal>>();
			shadowVertsG = new std::vector<std::vector<VertexGlobal>>();
			modelFaces = new std::vector<std::vector<Face>>();
			shadowFaces = new std::vector<std::vector<Face>>();
			assetSubfiles = new std::vector<std::string>();
			assetSubfileTypes = new std::vector<std::string>();
			assetSubfileStatus = new std::vector<bool>();
			focusedMsetItem = nullptr;
			this->Text = std2system(prog + " v " + version);
			msetTextureListView->MouseLeave += gcnew EventHandler(this, &BaseForm::msetTextureListView_Leave);
			msetTextureListView->MouseMove += gcnew MouseEventHandler(this, &BaseForm::msetTextureListView_MouseMove);
			msetTextureListView->MouseWheel += gcnew MouseEventHandler(this, &BaseForm::msetTextureListView_Scroll);
			msetNumericUpDown->ValueChanged += gcnew EventHandler(this, &BaseForm::msetNumericUpDown_ValueChanged);
			assetSubFileListView->ColumnWidthChanging += gcnew ColumnWidthChangingEventHandler(this, &BaseForm::subFileListView_ColumnWidthChanging);
			msetSubFileListView->ColumnWidthChanging += gcnew ColumnWidthChangingEventHandler(this, &BaseForm::subFileListView_ColumnWidthChanging);
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~BaseForm()
		{
			if (assetData)
			{
				delete assetData;
				assetData = NULL;
			}
			if (msetData)
			{
				delete msetData;
				msetData = NULL;
			}
			if (templateData)
			{
				delete templateData;
				templateData = NULL;
			}
			if (assetName)
			{
				delete assetName;
				assetName = NULL;
			}
			if (msetName)
			{
				delete msetName;
				msetName = NULL;
			}
			if (assetType)
			{
				delete assetType;
				assetType = NULL;
			}
			if (assetTextures)
			{
				delete assetTextures;
				assetTextures = NULL;
			}
			if (msetBaseTextures)
			{
				delete msetBaseTextures;
				msetBaseTextures = NULL;
			}
			if (msetTextures)
			{
				delete msetTextures;
				msetTextures = NULL;
			}
			if (refInds)
			{
				delete refInds;
				refInds = NULL;
			}
			if (modelJoints)
			{
				delete modelJoints;
				modelJoints = NULL;
			}
			if (modelVertsR)
			{
				delete modelVertsR;
				modelVertsR = NULL;
			}
			if (shadowVertsR)
			{
				delete shadowVertsR;
				shadowVertsR = NULL;
			}
			if (modelVertsG)
			{
				delete modelVertsG;
				modelVertsG = NULL;
			}
			if (shadowVertsG)
			{
				delete shadowVertsG;
				shadowVertsG = NULL;
			}
			if (modelFaces)
			{
				delete modelFaces;
				modelFaces = NULL;
			}
			if (shadowFaces)
			{
				delete shadowFaces;
				shadowFaces = NULL;
			}
			if (assetSubfiles)
			{
				delete assetSubfiles;
				assetSubfiles = NULL;
			}
			if (assetSubfileTypes)
			{
				delete assetSubfileTypes;
				assetSubfileTypes = NULL;
			}
			if (assetSubfileStatus)
			{
				delete assetSubfileStatus;
				assetSubfileStatus = NULL;
			}
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  importButton;
	private: System::Windows::Forms::Button^  exportButton;
	private: System::Windows::Forms::Button^  templateButton;
	private: System::Windows::Forms::RichTextBox^  importTextBox;
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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(BaseForm::typeid));
			this->importButton = (gcnew System::Windows::Forms::Button());
			this->exportButton = (gcnew System::Windows::Forms::Button());
			this->templateButton = (gcnew System::Windows::Forms::Button());
			this->importTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->templateTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->msetButton = (gcnew System::Windows::Forms::Button());
			this->msetTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->exportProgressBar = (gcnew System::Windows::Forms::ProgressBar());
			this->flipCheckBox = (gcnew System::Windows::Forms::CheckBox());
			this->yCheckBox = (gcnew System::Windows::Forms::CheckBox());
			this->normalCheckBox = (gcnew System::Windows::Forms::CheckBox());
			this->combineCheckBox = (gcnew System::Windows::Forms::CheckBox());
			this->assetTextureList = (gcnew System::Windows::Forms::ImageList(this->components));
			this->msetTextureList = (gcnew System::Windows::Forms::ImageList(this->components));
			this->assetTextureListView = (gcnew System::Windows::Forms::ListView());
			this->texture = (gcnew System::Windows::Forms::ColumnHeader());
			this->assetTextureListViewContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->importAssetImageToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportAssetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportAllAssetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->assignPaletteToPixelsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->addAssetImageToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeAssetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->msetTextureListView = (gcnew System::Windows::Forms::ListView());
			this->msetTextureListViewContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->importMsetImageToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportMsetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportAllMsetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->addMsetImageToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeMsetImagesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->msetNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->assetSubFileListView = (gcnew System::Windows::Forms::ListView());
			this->assetSubFilename = (gcnew System::Windows::Forms::ColumnHeader());
			this->assetSubFileType = (gcnew System::Windows::Forms::ColumnHeader());
			this->assetSubFileExists = (gcnew System::Windows::Forms::ColumnHeader());
			this->assetSubFileListViewContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->importAssetSubFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportAssetSubFilesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportAssetAllToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->msetSubFileListView = (gcnew System::Windows::Forms::ListView());
			this->msetSubFilename = (gcnew System::Windows::Forms::ColumnHeader());
			this->msetSubFileType = (gcnew System::Windows::Forms::ColumnHeader());
			this->msetSubFileExists = (gcnew System::Windows::Forms::ColumnHeader());
			this->msetSubFileListViewContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->importMsetSubFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportMsetSubFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exportMsetAllToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->assetTextureListViewContextMenu->SuspendLayout();
			this->msetTextureListViewContextMenu->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->msetNumericUpDown))->BeginInit();
			this->assetSubFileListViewContextMenu->SuspendLayout();
			this->msetSubFileListViewContextMenu->SuspendLayout();
			this->SuspendLayout();
			// 
			// importButton
			// 
			this->importButton->Location = System::Drawing::Point(9, 10);
			this->importButton->Margin = System::Windows::Forms::Padding(2);
			this->importButton->Name = L"importButton";
			this->importButton->Size = System::Drawing::Size(98, 24);
			this->importButton->TabIndex = 0;
			this->importButton->Text = L"Import Asset";
			this->importButton->UseVisualStyleBackColor = true;
			this->importButton->Click += gcnew System::EventHandler(this, &BaseForm::importButton_Click);
			// 
			// exportButton
			// 
			this->exportButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->exportButton->Location = System::Drawing::Point(9, 306);
			this->exportButton->Margin = System::Windows::Forms::Padding(2);
			this->exportButton->Name = L"exportButton";
			this->exportButton->Size = System::Drawing::Size(98, 24);
			this->exportButton->TabIndex = 1;
			this->exportButton->Text = L"Export Asset";
			this->exportButton->UseVisualStyleBackColor = true;
			this->exportButton->Click += gcnew System::EventHandler(this, &BaseForm::exportButton_Click);
			// 
			// templateButton
			// 
			this->templateButton->Location = System::Drawing::Point(9, 38);
			this->templateButton->Margin = System::Windows::Forms::Padding(2);
			this->templateButton->Name = L"templateButton";
			this->templateButton->Size = System::Drawing::Size(98, 24);
			this->templateButton->TabIndex = 2;
			this->templateButton->Text = L"Import Template";
			this->templateButton->UseVisualStyleBackColor = true;
			this->templateButton->Click += gcnew System::EventHandler(this, &BaseForm::templateButton_Click);
			// 
			// importTextBox
			// 
			this->importTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->importTextBox->Enabled = false;
			this->importTextBox->Location = System::Drawing::Point(112, 10);
			this->importTextBox->Margin = System::Windows::Forms::Padding(2);
			this->importTextBox->Name = L"importTextBox";
			this->importTextBox->Size = System::Drawing::Size(682, 27);
			this->importTextBox->TabIndex = 3;
			this->importTextBox->Text = L"";
			// 
			// templateTextBox
			// 
			this->templateTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->templateTextBox->Enabled = false;
			this->templateTextBox->Location = System::Drawing::Point(112, 38);
			this->templateTextBox->Margin = System::Windows::Forms::Padding(2);
			this->templateTextBox->Name = L"templateTextBox";
			this->templateTextBox->Size = System::Drawing::Size(682, 24);
			this->templateTextBox->TabIndex = 4;
			this->templateTextBox->Text = L"";
			// 
			// msetButton
			// 
			this->msetButton->Location = System::Drawing::Point(9, 67);
			this->msetButton->Margin = System::Windows::Forms::Padding(2);
			this->msetButton->Name = L"msetButton";
			this->msetButton->Size = System::Drawing::Size(98, 24);
			this->msetButton->TabIndex = 5;
			this->msetButton->Text = L"Import MSET";
			this->msetButton->UseVisualStyleBackColor = true;
			this->msetButton->Click += gcnew System::EventHandler(this, &BaseForm::msetButton_Click);
			// 
			// msetTextBox
			// 
			this->msetTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->msetTextBox->Enabled = false;
			this->msetTextBox->Location = System::Drawing::Point(112, 67);
			this->msetTextBox->Margin = System::Windows::Forms::Padding(2);
			this->msetTextBox->Name = L"msetTextBox";
			this->msetTextBox->Size = System::Drawing::Size(682, 24);
			this->msetTextBox->TabIndex = 6;
			this->msetTextBox->Text = L"";
			// 
			// exportProgressBar
			// 
			this->exportProgressBar->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->exportProgressBar->Location = System::Drawing::Point(431, 306);
			this->exportProgressBar->Margin = System::Windows::Forms::Padding(2);
			this->exportProgressBar->Name = L"exportProgressBar";
			this->exportProgressBar->Size = System::Drawing::Size(362, 24);
			this->exportProgressBar->TabIndex = 7;
			this->exportProgressBar->Visible = false;
			// 
			// flipCheckBox
			// 
			this->flipCheckBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->flipCheckBox->AutoSize = true;
			this->flipCheckBox->Enabled = false;
			this->flipCheckBox->Location = System::Drawing::Point(112, 310);
			this->flipCheckBox->Margin = System::Windows::Forms::Padding(2);
			this->flipCheckBox->Name = L"flipCheckBox";
			this->flipCheckBox->Size = System::Drawing::Size(65, 17);
			this->flipCheckBox->TabIndex = 8;
			this->flipCheckBox->Text = L"Flip UVs";
			this->flipCheckBox->UseVisualStyleBackColor = true;
			// 
			// yCheckBox
			// 
			this->yCheckBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->yCheckBox->AutoSize = true;
			this->yCheckBox->Enabled = false;
			this->yCheckBox->Location = System::Drawing::Point(176, 310);
			this->yCheckBox->Margin = System::Windows::Forms::Padding(2);
			this->yCheckBox->Name = L"yCheckBox";
			this->yCheckBox->Size = System::Drawing::Size(72, 17);
			this->yCheckBox->TabIndex = 9;
			this->yCheckBox->Text = L"Y Axis Up";
			this->yCheckBox->UseVisualStyleBackColor = true;
			// 
			// normalCheckBox
			// 
			this->normalCheckBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->normalCheckBox->AutoSize = true;
			this->normalCheckBox->Enabled = false;
			this->normalCheckBox->Location = System::Drawing::Point(248, 310);
			this->normalCheckBox->Margin = System::Windows::Forms::Padding(2);
			this->normalCheckBox->Name = L"normalCheckBox";
			this->normalCheckBox->Size = System::Drawing::Size(72, 17);
			this->normalCheckBox->TabIndex = 10;
			this->normalCheckBox->Text = L"Normalize";
			this->normalCheckBox->UseVisualStyleBackColor = true;
			// 
			// combineCheckBox
			// 
			this->combineCheckBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->combineCheckBox->AutoSize = true;
			this->combineCheckBox->Enabled = false;
			this->combineCheckBox->Location = System::Drawing::Point(323, 310);
			this->combineCheckBox->Margin = System::Windows::Forms::Padding(2);
			this->combineCheckBox->Name = L"combineCheckBox";
			this->combineCheckBox->Size = System::Drawing::Size(107, 17);
			this->combineCheckBox->TabIndex = 11;
			this->combineCheckBox->Text = L"Combine Meshes";
			this->combineCheckBox->UseVisualStyleBackColor = true;
			// 
			// assetTextureList
			// 
			this->assetTextureList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
			this->assetTextureList->ImageSize = System::Drawing::Size(128, 128);
			this->assetTextureList->TransparentColor = System::Drawing::Color::Transparent;
			// 
			// msetTextureList
			// 
			this->msetTextureList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
			this->msetTextureList->ImageSize = System::Drawing::Size(128, 64);
			this->msetTextureList->TransparentColor = System::Drawing::Color::Transparent;
			// 
			// assetTextureListView
			// 
			this->assetTextureListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left));
			this->assetTextureListView->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(1) { this->texture });
			this->assetTextureListView->ContextMenuStrip = this->assetTextureListViewContextMenu;
			this->assetTextureListView->Enabled = false;
			this->assetTextureListView->HideSelection = false;
			this->assetTextureListView->LargeImageList = this->assetTextureList;
			this->assetTextureListView->Location = System::Drawing::Point(9, 93);
			this->assetTextureListView->Margin = System::Windows::Forms::Padding(2);
			this->assetTextureListView->Name = L"assetTextureListView";
			this->assetTextureListView->Size = System::Drawing::Size(193, 209);
			this->assetTextureListView->TabIndex = 12;
			this->assetTextureListView->UseCompatibleStateImageBehavior = false;
			// 
			// texture
			// 
			this->texture->Text = L"Model Textures";
			// 
			// assetTextureListViewContextMenu
			// 
			this->assetTextureListViewContextMenu->ImageScalingSize = System::Drawing::Size(20, 20);
			this->assetTextureListViewContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(6) {
				this->importAssetImageToolStripMenuItem,
					this->exportAssetImagesToolStripMenuItem, this->exportAllAssetImagesToolStripMenuItem, this->assignPaletteToPixelsToolStripMenuItem,
					this->addAssetImageToolStripMenuItem, this->removeAssetImagesToolStripMenuItem
			});
			this->assetTextureListViewContextMenu->Name = L"contextMenuStrip1";
			this->assetTextureListViewContextMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
			this->assetTextureListViewContextMenu->Size = System::Drawing::Size(196, 136);
			// 
			// importAssetImageToolStripMenuItem
			// 
			this->importAssetImageToolStripMenuItem->Name = L"importAssetImageToolStripMenuItem";
			this->importAssetImageToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->importAssetImageToolStripMenuItem->Text = L"Import Image";
			this->importAssetImageToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::importAssetImageToolStripMenuItem_Click);
			// 
			// exportAssetImagesToolStripMenuItem
			// 
			this->exportAssetImagesToolStripMenuItem->Name = L"exportAssetImagesToolStripMenuItem";
			this->exportAssetImagesToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->exportAssetImagesToolStripMenuItem->Text = L"Export Image(s)";
			this->exportAssetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportAssetImagesToolStripMenuItem_Click);
			// 
			// exportAllAssetImagesToolStripMenuItem
			// 
			this->exportAllAssetImagesToolStripMenuItem->Name = L"exportAllAssetImagesToolStripMenuItem";
			this->exportAllAssetImagesToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->exportAllAssetImagesToolStripMenuItem->Text = L"Export All";
			this->exportAllAssetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportAllAssetImagesToolStripMenuItem_Click);
			// 
			// assignPaletteToPixelsToolStripMenuItem
			// 
			this->assignPaletteToPixelsToolStripMenuItem->Name = L"assignPaletteToPixelsToolStripMenuItem";
			this->assignPaletteToPixelsToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->assignPaletteToPixelsToolStripMenuItem->Text = L"Assign Palette to Pixels";
			this->assignPaletteToPixelsToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::assignPaletteToPixelsToolStripMenuItem_Click);
			// 
			// addAssetImageToolStripMenuItem
			// 
			this->addAssetImageToolStripMenuItem->Name = L"addAssetImageToolStripMenuItem";
			this->addAssetImageToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->addAssetImageToolStripMenuItem->Text = L"Add Image";
			this->addAssetImageToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::addAssetImageToolStripMenuItem_Click);
			// 
			// removeAssetImagesToolStripMenuItem
			// 
			this->removeAssetImagesToolStripMenuItem->Name = L"removeAssetImagesToolStripMenuItem";
			this->removeAssetImagesToolStripMenuItem->Size = System::Drawing::Size(195, 22);
			this->removeAssetImagesToolStripMenuItem->Text = L"Remove Image(s)";
			this->removeAssetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::removeAssetImagesToolStripMenuItem_Click);
			// 
			// msetTextureListView
			// 
			this->msetTextureListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left));
			this->msetTextureListView->ContextMenuStrip = this->msetTextureListViewContextMenu;
			this->msetTextureListView->Enabled = false;
			this->msetTextureListView->HideSelection = false;
			this->msetTextureListView->LargeImageList = this->msetTextureList;
			this->msetTextureListView->Location = System::Drawing::Point(206, 93);
			this->msetTextureListView->Margin = System::Windows::Forms::Padding(2);
			this->msetTextureListView->Name = L"msetTextureListView";
			this->msetTextureListView->Size = System::Drawing::Size(193, 209);
			this->msetTextureListView->TabIndex = 13;
			this->msetTextureListView->UseCompatibleStateImageBehavior = false;
			// 
			// msetTextureListViewContextMenu
			// 
			this->msetTextureListViewContextMenu->ImageScalingSize = System::Drawing::Size(20, 20);
			this->msetTextureListViewContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
				this->importMsetImageToolStripMenuItem,
					this->exportMsetImagesToolStripMenuItem, this->exportAllMsetImagesToolStripMenuItem, this->addMsetImageToolStripMenuItem, this->removeMsetImagesToolStripMenuItem
			});
			this->msetTextureListViewContextMenu->Name = L"msetTextureListViewContextMenu";
			this->msetTextureListViewContextMenu->Size = System::Drawing::Size(167, 114);
			// 
			// importMsetImageToolStripMenuItem
			// 
			this->importMsetImageToolStripMenuItem->Name = L"importMsetImageToolStripMenuItem";
			this->importMsetImageToolStripMenuItem->Size = System::Drawing::Size(166, 22);
			this->importMsetImageToolStripMenuItem->Text = L"Import Image";
			this->importMsetImageToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::importMsetImageToolStripMenuItem_Click);
			// 
			// exportMsetImagesToolStripMenuItem
			// 
			this->exportMsetImagesToolStripMenuItem->Name = L"exportMsetImagesToolStripMenuItem";
			this->exportMsetImagesToolStripMenuItem->Size = System::Drawing::Size(166, 22);
			this->exportMsetImagesToolStripMenuItem->Text = L"Export Image(s)";
			this->exportMsetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportMsetImagesToolStripMenuItem_Click);
			// 
			// exportAllMsetImagesToolStripMenuItem
			// 
			this->exportAllMsetImagesToolStripMenuItem->Name = L"exportAllMsetImagesToolStripMenuItem";
			this->exportAllMsetImagesToolStripMenuItem->Size = System::Drawing::Size(166, 22);
			this->exportAllMsetImagesToolStripMenuItem->Text = L"Export All";
			this->exportAllMsetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportAllMsetImagesToolStripMenuItem_Click);
			// 
			// addMsetImageToolStripMenuItem
			// 
			this->addMsetImageToolStripMenuItem->Name = L"addMsetImageToolStripMenuItem";
			this->addMsetImageToolStripMenuItem->Size = System::Drawing::Size(166, 22);
			this->addMsetImageToolStripMenuItem->Text = L"Add Image";
			this->addMsetImageToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::addMsetImageToolStripMenuItem_Click);
			// 
			// removeMsetImagesToolStripMenuItem
			// 
			this->removeMsetImagesToolStripMenuItem->Name = L"removeMsetImagesToolStripMenuItem";
			this->removeMsetImagesToolStripMenuItem->Size = System::Drawing::Size(166, 22);
			this->removeMsetImagesToolStripMenuItem->Text = L"Remove Image(s)";
			this->removeMsetImagesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::removeMsetImagesToolStripMenuItem_Click);
			// 
			// msetNumericUpDown
			// 
			this->msetNumericUpDown->Enabled = false;
			this->msetNumericUpDown->Location = System::Drawing::Point(323, 95);
			this->msetNumericUpDown->Margin = System::Windows::Forms::Padding(2);
			this->msetNumericUpDown->Name = L"msetNumericUpDown";
			this->msetNumericUpDown->ReadOnly = true;
			this->msetNumericUpDown->Size = System::Drawing::Size(74, 20);
			this->msetNumericUpDown->TabIndex = 14;
			this->msetNumericUpDown->Visible = false;
			// 
			// assetSubFileListView
			// 
			this->assetSubFileListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->assetSubFileListView->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
				this->assetSubFilename,
					this->assetSubFileType, this->assetSubFileExists
			});
			this->assetSubFileListView->ContextMenuStrip = this->assetSubFileListViewContextMenu;
			this->assetSubFileListView->Enabled = false;
			this->assetSubFileListView->FullRowSelect = true;
			this->assetSubFileListView->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
			this->assetSubFileListView->HideSelection = false;
			this->assetSubFileListView->Location = System::Drawing::Point(403, 93);
			this->assetSubFileListView->Margin = System::Windows::Forms::Padding(2);
			this->assetSubFileListView->Name = L"assetSubFileListView";
			this->assetSubFileListView->Size = System::Drawing::Size(193, 209);
			this->assetSubFileListView->TabIndex = 15;
			this->assetSubFileListView->UseCompatibleStateImageBehavior = false;
			this->assetSubFileListView->View = System::Windows::Forms::View::List;
			// 
			// assetSubFilename
			// 
			this->assetSubFilename->Text = L"Sub-Filename";
			this->assetSubFilename->Width = 107;
			// 
			// assetSubFileType
			// 
			this->assetSubFileType->Text = L"Type";
			this->assetSubFileType->Width = 42;
			// 
			// assetSubFileExists
			// 
			this->assetSubFileExists->Text = L"Exists";
			this->assetSubFileExists->Width = 40;
			// 
			// assetSubFileListViewContextMenu
			// 
			this->assetSubFileListViewContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->importAssetSubFileToolStripMenuItem,
					this->exportAssetSubFilesToolStripMenuItem, this->exportAssetAllToolStripMenuItem
			});
			this->assetSubFileListViewContextMenu->Name = L"subFileListViewContextMenu";
			this->assetSubFileListViewContextMenu->Size = System::Drawing::Size(168, 70);
			// 
			// importAssetSubFileToolStripMenuItem
			// 
			this->importAssetSubFileToolStripMenuItem->Name = L"importAssetSubFileToolStripMenuItem";
			this->importAssetSubFileToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->importAssetSubFileToolStripMenuItem->Text = L"Import Sub-File";
			this->importAssetSubFileToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::importAssetSubFileToolStripMenuItem_Click);
			// 
			// exportAssetSubFilesToolStripMenuItem
			// 
			this->exportAssetSubFilesToolStripMenuItem->Name = L"exportAssetSubFilesToolStripMenuItem";
			this->exportAssetSubFilesToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->exportAssetSubFilesToolStripMenuItem->Text = L"Export Sub-File(s)";
			this->exportAssetSubFilesToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportAssetSubFilesToolStripMenuItem_Click);
			// 
			// exportAssetAllToolStripMenuItem
			// 
			this->exportAssetAllToolStripMenuItem->Name = L"exportAssetAllToolStripMenuItem";
			this->exportAssetAllToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->exportAssetAllToolStripMenuItem->Text = L"Export All";
			this->exportAssetAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportAssetAllToolStripMenuItem_Click);
			// 
			// msetSubFileListView
			// 
			this->msetSubFileListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->msetSubFileListView->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
				this->msetSubFilename,
					this->msetSubFileType, this->msetSubFileExists
			});
			this->msetSubFileListView->ContextMenuStrip = this->msetSubFileListViewContextMenu;
			this->msetSubFileListView->Enabled = false;
			this->msetSubFileListView->FullRowSelect = true;
			this->msetSubFileListView->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
			this->msetSubFileListView->HideSelection = false;
			this->msetSubFileListView->Location = System::Drawing::Point(600, 93);
			this->msetSubFileListView->Margin = System::Windows::Forms::Padding(2);
			this->msetSubFileListView->Name = L"msetSubFileListView";
			this->msetSubFileListView->Size = System::Drawing::Size(193, 209);
			this->msetSubFileListView->TabIndex = 16;
			this->msetSubFileListView->UseCompatibleStateImageBehavior = false;
			this->msetSubFileListView->View = System::Windows::Forms::View::List;
			this->msetSubFileListView->Visible = false;
			// 
			// msetSubFilename
			// 
			this->msetSubFilename->Text = L"Sub-Filename";
			this->msetSubFilename->Width = 109;
			// 
			// msetSubFileType
			// 
			this->msetSubFileType->Text = L"Type";
			this->msetSubFileType->Width = 40;
			// 
			// msetSubFileExists
			// 
			this->msetSubFileExists->Text = L"Exists";
			this->msetSubFileExists->Width = 40;
			// 
			// msetSubFileListViewContextMenu
			// 
			this->msetSubFileListViewContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->importMsetSubFileToolStripMenuItem,
					this->exportMsetSubFileToolStripMenuItem, this->exportMsetAllToolStripMenuItem
			});
			this->msetSubFileListViewContextMenu->Name = L"subFileListViewContextMenu";
			this->msetSubFileListViewContextMenu->Size = System::Drawing::Size(168, 70);
			// 
			// importMsetSubFileToolStripMenuItem
			// 
			this->importMsetSubFileToolStripMenuItem->Name = L"importMsetSubFileToolStripMenuItem";
			this->importMsetSubFileToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->importMsetSubFileToolStripMenuItem->Text = L"Import Sub-File";
			this->importMsetSubFileToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::importMsetSubFileToolStripMenuItem_Click);
			// 
			// exportMsetSubFileToolStripMenuItem
			// 
			this->exportMsetSubFileToolStripMenuItem->Name = L"exportMsetSubFileToolStripMenuItem";
			this->exportMsetSubFileToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->exportMsetSubFileToolStripMenuItem->Text = L"Export Sub-File(s)";
			this->exportMsetSubFileToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportMsetSubFileToolStripMenuItem_Click);
			// 
			// exportMsetAllToolStripMenuItem
			// 
			this->exportMsetAllToolStripMenuItem->Name = L"exportMsetAllToolStripMenuItem";
			this->exportMsetAllToolStripMenuItem->Size = System::Drawing::Size(167, 22);
			this->exportMsetAllToolStripMenuItem->Text = L"Export All";
			this->exportMsetAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &BaseForm::exportMsetAllToolStripMenuItem_Click);
			// 
			// BaseForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(802, 340);
			this->Controls->Add(this->msetSubFileListView);
			this->Controls->Add(this->assetSubFileListView);
			this->Controls->Add(this->msetNumericUpDown);
			this->Controls->Add(this->msetTextureListView);
			this->Controls->Add(this->assetTextureListView);
			this->Controls->Add(this->combineCheckBox);
			this->Controls->Add(this->normalCheckBox);
			this->Controls->Add(this->yCheckBox);
			this->Controls->Add(this->flipCheckBox);
			this->Controls->Add(this->exportProgressBar);
			this->Controls->Add(this->msetTextBox);
			this->Controls->Add(this->msetButton);
			this->Controls->Add(this->templateTextBox);
			this->Controls->Add(this->importTextBox);
			this->Controls->Add(this->templateButton);
			this->Controls->Add(this->exportButton);
			this->Controls->Add(this->importButton);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Margin = System::Windows::Forms::Padding(2);
			this->Name = L"BaseForm";
			this->Text = L"HyperCrown";
			this->assetTextureListViewContextMenu->ResumeLayout(false);
			this->msetTextureListViewContextMenu->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->msetNumericUpDown))->EndInit();
			this->assetSubFileListViewContextMenu->ResumeLayout(false);
			this->msetSubFileListViewContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void importButton_Click(System::Object^  sender, System::EventArgs^  e) {
		bool success = true;
		OpenFileDialog fd;
		fd.Filter = L"ARD (*.ard)|*.ard|World Binary Files (*.bin)|*.bin|Collada (*.dae)|*.dae|Special Effects (*.dpx)|*.dpx|IMG (*.img)|*.img|Magic (*.mag)|*.mag|Character Model (*.mdls)|*.mdls|ARD MFA (*.mfa)|*.mfa|MLB (*.mlb)|*.mlb|ARD MOA (*.moa)|*.moa|PS2 (*.ps2)|*.ps2|RTB (*.rtb)|*.rtb|SE (*.se)|*.se|SPE (*.spe)|*.spe|TARC|*.tarc|TEZB (*.tzb)|*.tzb|VSB (*.vsb)|*.vsb|VSET (*.vset)|*.vset|WD (*.wd)|*.wd|Weapon Model (*.wpn)|*.wpn";
		fd.FilterIndex = 0;
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			std::string assetFileName = formatInputStr(system2std(fd.FileName)), iStr;
			std::stringstream ss;
			bool valid;
			// Clear data structures
			modelVertsG->clear();
			modelVertsR->clear();
			modelFaces->clear();
			modelJoints->clear();
			assetTextures->clear();
			shadowVertsG->clear();
			shadowVertsR->clear();
			shadowFaces->clear();
			assetSubfiles->clear();
			assetSubfileTypes->clear();
			assetSubfileStatus->clear();
			assetTextureList->Images->Clear();
			assetTextureListView->Items->Clear();
			assetSubFileListView->Items->Clear();
			combineCheckBox->Enabled = false;
			flipCheckBox->Enabled = false;
			normalCheckBox->Enabled = false;
			yCheckBox->Enabled = false;
			assetTextureListView->Enabled = false;
			assetSubFileListView->Enabled = false;
			assetSubFileListView->View = View::List;
			// Read from the file
			if (binaryFileRead(assetFileName, *assetData))
			{
				Assimp::Importer importer;
				const aiScene *scene;
				unsigned int assimpImporterFlags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_GenNormals | aiProcess_SortByPType | aiProcess_GenUVCoords;
				// Get the index of the last . in the filename
				int extInd = assetFileName.find_first_of('.'), nameStartInd = assetFileName.find_last_of('/');
				if (nameStartInd == assetFileName.size())
				{
					nameStartInd = 0;
				}
				else
				{
					nameStartInd++;
				}
				*assetName = assetFileName.substr(nameStartInd, extInd - nameStartInd);
				// Extract the file extension
				*assetType = assetFileName.substr(extInd, assetFileName.size() - extInd);
				if (*assetType == ".dae" || *assetType == ".mdls" || *assetType == ".mfa" || *assetType == ".mlb" || *assetType == ".moa" || *assetType == ".wpn")
				{
					combineCheckBox->Enabled = true;
					flipCheckBox->Enabled = true;
					normalCheckBox->Enabled = true;
					yCheckBox->Enabled = true;
				}
				// Set the textbox's value to the filename to give the user feedback
				importTextBox->Text = std2system(assetFileName);
				// Determine the type of model based on the extension
				if (*assetType == ".ard")
				{
					std::vector<std::string> tmpFiles = getArdMoas(*assetData);
					assetSubfiles->reserve(2);
					assetSubfileTypes->reserve(2);
					assetSubfileStatus->reserve(2);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("MOAs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					tmpFiles = getArdMfas(*assetData);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("MFAs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("MOAs");
					assetSubfileTypes->push_back("MFAs");
					
				}
				else if (*assetType == ".bin")
				{
					if (getBasicJoints(*modelJoints))
					{
						if (getBinVerticesAndFaces(*assetData, *modelVertsG, *modelFaces))
						{
							if (getVerticesRelative(*modelJoints, *modelVertsG, *modelVertsR))
							{
								if (getBinCollisions(*assetData, *shadowVertsG, *shadowFaces))
								{
									if (getVerticesRelative(*modelJoints, *shadowVertsG, *shadowVertsR))
									{
										*assetTextures = getBinTextures(*assetData);
										assetSubfiles->reserve(2);
										assetSubfileTypes->reserve(2);
										assetSubfileStatus->reserve(2);
										assetSubfiles->push_back(getBinDpx(*assetData, 0));
										assetSubfileStatus->push_back(assetSubfiles->back() != "");
										assetSubfiles->push_back(getBinDpx(*assetData, 1));
										assetSubfileStatus->push_back(assetSubfiles->back() != "");
										assetSubfileTypes->push_back("DPX");
										assetSubfileTypes->push_back("DPX");
									}
									else
									{
										MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
											MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
										success = false;
									}
								}
								else
								{
									MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
										MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
									success = false;
								}
							}
							else
							{
								MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}
						}
						else
						{
							MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
				}
				else if (*assetType == ".dae")
				{
					// Use Assimp to get import the DAE
					importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, PP_RVC_FLAGS);
					scene = importer.ReadFileFromMemory(assetData->c_str(), assetData->size(), assimpImporterFlags);
					if (getSceneJoints(scene, *modelJoints, 1))
					{
						if (getSceneVerticesAndFaces(scene, *modelVertsG, *modelFaces, *modelJoints))
						{
							if (getVerticesRelative(*modelJoints, *modelVertsG, *modelVertsR))
							{
								std::string file_path = system2std(importTextBox->Text);
								file_path = file_path.substr(0, file_path.find_last_of('/') + 1);
								*assetTextures = getSceneTextures(scene, file_path);
							}
							else
							{
								MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}
						}
						else
						{
							MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
				}
				else if (*assetType == ".dpx")
				{
					std::vector<std::string> tmpFiles = getDpxSpecialEffects(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("SPEs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("SPEs");
				}
				else if (*assetType == ".img")
				{
					*assetTextures = getImgTextures(*assetData);
				}
				else if (*assetType == ".mag")
				{
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					assetSubfiles->push_back(getMagDpx(*assetData));
					assetSubfileTypes->push_back("DPX");
					assetSubfileStatus->push_back(assetSubfiles->back() != "");
				}
				else if (*assetType == ".mlb")
				{
					if (getBasicJoints(*modelJoints))
					{
						if (getMlbVerticesAndFaces(*assetData, *modelVertsR, *modelFaces))
						{
							if (!getVerticesGlobal(*modelJoints, *modelVertsR, *modelVertsG))
							{
								MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}
						}
						else
						{
							MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
				}
				else if (*assetType == ".mdls")
				{
					// Get all of the currently extractable contents of a MDLS
					std::vector<Texture> tmpTex;
					std::vector<std::string> tmpFiles;
					*modelJoints = getMdlsJoints(*assetData, absoluteMaxJoints);
					*assetTextures = getMdlsTextures(*assetData);
					assetSubfiles->reserve(4);
					assetSubfileTypes->reserve(4);
					assetSubfileStatus->reserve(4);
					assetSubfiles->push_back(getMdlsDpx(*assetData));
					assetSubfileStatus->push_back(assetSubfiles->back() != "");
					tmpTex = getMdlsHUDTextures(*assetData);
					if (tmpTex.size() > 0)
					{
						assetSubfiles->push_back("TIM2");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfiles->push_back(getMdlsSe(*assetData));
					assetSubfileStatus->push_back(assetSubfiles->back() != "");
					tmpFiles = getMdlsVags(*assetData);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("VAGs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("DPX");
					assetSubfileTypes->push_back("TIM2");
					assetSubfileTypes->push_back("SE");
					assetSubfileTypes->push_back("VAGs");

					valid = getMdlsVerticesAndFaces(*assetData, *modelJoints, *modelVertsR, *modelFaces);
					valid = valid && getMdlsShadowVerticesAndFaces(*assetData, *modelJoints, *shadowVertsR, *shadowFaces);
					if (valid)
					{
						valid = getVerticesGlobal(*modelJoints, *modelVertsR, *modelVertsG);
						valid = valid && getVerticesGlobal(*modelJoints, *shadowVertsR, *shadowVertsG);
						if (!valid)
						{
							MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					if (!success)
					{
						assetSubfiles->clear();
						assetSubfileTypes->clear();
						assetSubFileListView->Items->Clear();
					}
				}
				else if (*assetType == ".mfa")
				{
					*assetTextures = getMfaTextures(*assetData);
				}
				else if (*assetType == ".moa")
				{
					*modelJoints = getMoaJoints(*assetData);
					if (getMoaVerticesAndFaces(*assetData, *modelJoints, *modelVertsR, *modelFaces))
					{
						if (getVerticesGlobal(*modelJoints, *modelVertsR, *modelVertsG))
						{
							if (getMoaShadowVerticesAndFaces(*assetData, *modelJoints, *shadowVertsR, *shadowFaces))
							{
								if (getVerticesGlobal(*modelJoints, *shadowVertsR, *shadowVertsG))
								{
									*assetTextures = getMoaTextures(*assetData);
									assetSubfiles->reserve(1);
									assetSubfileTypes->reserve(1);
									assetSubfileStatus->reserve(1);
									assetSubfiles->push_back(getMoaDpx(*assetData));
									assetSubfileStatus->push_back(assetSubfiles->back() != "");
									assetSubfileTypes->push_back("DPX");
								}
							}
						}
					}
				}
				else if (*assetType == ".ps2")
				{
					*assetTextures = getPs2Textures(*assetData);
				}
				else if (*assetType == ".rtb" || *assetType == ".tarc")
				{
					std::vector<std::string> tmpFiles = getTarcFiles(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("FILEs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("FILEs");
				}
				else if (*assetType == ".se")
				{
					std::vector<std::string> tmpFiles = getSeWds(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back(tmpFiles[0]);
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("WD");
				}
				else if (*assetType == ".spe")
				{
					*assetTextures = getSpeTextures(*assetData);
				}
				else if (*assetType == ".tzb")
				{
					*assetTextures = getTezbTextures(*assetData);
				}
				else if (*assetType == ".vsb")
				{
					std::vector<std::string> tmpFiles = getVsbVags(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("VAGs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("VAGs");
				}
				else if (*assetType == ".vset")
				{
					std::vector<std::string> tmpFiles = getVsetVags(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("VAGs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("VAGs");
				}
				else if (*assetType == ".wd")
				{
					std::vector<std::string> tmpFiles = getWdVags(*assetData, *assetName);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					if (tmpFiles.size() > 0)
					{
						assetSubfiles->push_back("VAGs");
						assetSubfileStatus->push_back(true);
					}
					else
					{
						assetSubfiles->push_back("");
						assetSubfileStatus->push_back(false);
					}
					assetSubfileTypes->push_back("VAGs");
				}
				else if (*assetType == ".wpn")
				{
					// Get all of the currently extractable contents of a WPN
					*modelJoints = getWpnJoints(*assetData, "", absoluteMaxJoints);
					*assetTextures = getWpnTextures(*assetData);
					assetSubfiles->reserve(1);
					assetSubfileTypes->reserve(1);
					assetSubfileStatus->reserve(1);
					assetSubfiles->push_back(getWpnDpx(*assetData));
					assetSubfileStatus->push_back(assetSubfiles->back() != "");
					assetSubfileTypes->push_back("DPX");

					if (getWpnVerticesAndFaces(*assetData, *modelVertsR, *modelFaces))
					{
						if (!getVerticesGlobal(*modelJoints, *modelVertsR, *modelVertsG))
						{
							MessageBox::Show(this, L"Failed to Convert Model Vertices", L"HyperCrown - Import Model",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to Import Model", L"HyperCrown - Import Model",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}

					if (!success)
					{
						assetSubfiles->clear();
						assetSubfileTypes->clear();
						assetSubFileListView->Items->Clear();
					}
				}
				else
				{
					MessageBox::Show(this, L"Unsupported Model Type", L"HyperCrown - Import Model",
						MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					success = false;
				}
				if (*assetData != "")
				{
					// For each texture...
					for (unsigned int i = 0; i < assetTextures->size(); i++)
					{
						// Get texture i
						Texture ti = (*assetTextures)[i];
						// Intialize the bitmap
						Bitmap ^bmi;
						ss.clear();
						ss.str("");
						ss << i;
						ss >> iStr;
						// Convert the texture to a bitmap
						bmi = textureToBitmap(ti);
						// Populate the asset texture list
						// If this is the first image, use its dims
						if (i == 0)
						{
							unsigned int width = ti.width, height = ti.height;
							if (width > 128)
							{
								width = 128;
							}
							if (height > 128)
							{
								height = 128;
							}
							assetTextureList->ImageSize = Drawing::Size(width, height);
						}
						assetTextureList->Images->Add(bmi);
						assetTextureListView->Items->Add(gcnew ListViewItem(std2system(*assetName + "-" + iStr), i));
					}
					// Check if we need to update mset textures
					if (msetTextures->size() > 0)
					{
						if (assetTextures->size() > 0)
						{
							msetTextureList->Images->Clear();
							// For each mset texture
							for(unsigned int i = 0; i < msetTextures->size(); i++)
							{
								unsigned int refInd = (*refInds)[i];
								// Store the texture
								Texture t, mt = (*msetTextures)[i];
								// Intialize the bitmap
								Bitmap ^bmi;
								if (refInd >= assetTextures->size())
								{
									refInd = 0;
								}
								t = (*assetTextures)[refInd];
								// Update the texture palette to match the imported image
								mt.palette = t.palette;
								// Convert the texture to a bitmap
								bmi = textureToBitmap(mt);
								// Replace the mset image
								(*msetTextures)[i] = mt;
								msetTextureList->Images->Add(bmi);
							}
						}
						else
						{
							// There are no textures, so we can't have mset textures
							msetTextures->clear();
							msetTextureList->Images->Clear();
							msetTextureListView->Items->Clear();
						}
					}
					for (unsigned int i = 0; i < assetSubfiles->size(); i++)
					{
						ListViewItem ^itm = gcnew ListViewItem();
						ss.clear();
						ss.str("");
						ss << i;
						ss >> iStr;
						itm->Text = std2system(*assetName + "-s" + iStr);
						itm->UseItemStyleForSubItems = true;
						itm->SubItems->Add(std2system((*assetSubfileTypes)[i]));
						if ((*assetSubfileStatus)[i])
						{
							itm->SubItems->Add(L"True");
						}
						else
						{
							itm->SubItems->Add(L"False");
							itm->BackColor = Color::LightGray;
							IEnumerator ^enm = itm->SubItems->GetEnumerator();
							while (enm->MoveNext())
							{
								((ListViewItem::ListViewSubItem^)enm->Current)->BackColor = Color::LightGray;
							}
						}
						assetSubFileListView->Items->Add(itm);
					}
					if (assetTextureListView->Items->Count > 0 || modelVertsR->size() > 0)
					{
						assetTextureListView->Enabled = true;
					}
					if (assetSubFileListView->Items->Count > 0)
					{
						assetSubFileListView->View = View::Details;
						assetSubFileListView->Enabled = true;
					}
				}
			}
			else
			{
				MessageBox::Show(this, L"Failed to Read From File", L"HyperCrown - Import Asset",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
				success = false;
			}
		}
		if (!success)
		{
			importTextBox->Text = L"";
			*assetData = "";
			msetTextBox->Text = L"";
			*msetData = "";
			msetTextures->clear();
			msetTextureList->Images->Clear();
			msetTextureListView->Items->Clear();
			msetSubFileListView->Items->Clear();
			msetTextureListView->Enabled = false;
			msetSubFileListView->Enabled = false;
		}
		return;
	}

	private: System::Void exportButton_Click(System::Object^  sender, System::EventArgs^  e) {
		bool successful = true;
		SaveFileDialog fd;
		fd.Filter = L"";
		fd.FilterIndex = 0;
		fd.AddExtension = true;
		if (*assetType == ".ard")
		{
			fd.Filter = L"ARD (*.ard)|*.ard";
		}
		else if (*assetType == ".bin" || *assetType == ".dae" || *assetType == ".mdls" || *assetType == ".mfa" || *assetType == ".mlb" || *assetType == ".moa" || *assetType == ".wpn")
		{
			fd.Filter = L"Collada (*.dae)|*.dae|Character Model (*.mdls)|*.mdls|OBJ (*.obj)|*.obj|Weapon Model (*.wpn)|*.wpn";
		}
		else if (*assetType == ".dpx")
		{
			fd.Filter = L"DPX (*.dpx)|*.dpx";
		}
		else if (*assetType == ".mag")
		{
			fd.Filter = L"MAG (*.mag)|*.mag";
		}
		else if (*assetType == ".ps2")
		{
			fd.Filter = L"PS2 (*.ps2)|*.ps2";
		}
		else if (*assetType == ".rtb" || *assetType == ".tarc")
		{
			fd.Filter = L"RTB (*.rtb)|*.rtb|TARC (*.tarc)|*.tarc";
		}
		else if (*assetType == ".se")
		{
			fd.Filter = L"SE (*.se)|*.se";
		}
		else if (*assetType == ".spe")
		{
			fd.Filter = L"SPE (*.spe)|*.spe";
		}
		else if (*assetType == ".tzb")
		{
			fd.Filter = L"TEZB (*.tzb)|*.tzb";
		}
		else if (*assetType == ".vsb" || *assetType == ".vset" || *assetType == ".wd")
		{
			if (*assetType == ".vsb")
			{
				fd.Filter = L"VSB (*.vsb)|*.vsb";
			}
			if (fd.Filter != L"")
			{
				fd.Filter += L"|VSET (*.vset)|*.vset";
			}
			else
			{
				fd.Filter += L"VSET (*.vset)|*.vset";
			}
			if (*assetType == ".wd")
			{
				fd.Filter += L"|WD (*.wd)|*.wd";
			}
		}
		if (*msetData != "")
		{
			if (fd.Filter != L"")
			{
				fd.Filter += L"|Character Moveset (*.mset)|*.mset";
			}
			else
			{
				fd.Filter += L"Character Moveset (*.mset)|*.mset";
			}
		}
		if (fd.Filter == L"")
		{
			if (*assetType == "")
			{
				MessageBox::Show(this, L"Nothing to Export", L"HyperCrown - Export",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
			else
			{
				MessageBox::Show(this, L"Unable to Export the Asset", L"HyperCrown - Export",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
			return;
		}
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			unsigned int extStartInd;
			int exportType = -1;
			std::string outName = formatInputStr(system2std(fd.FileName)), outType;
			extStartInd = outName.find_last_of('.');
			outType = outName.substr(extStartInd, outName.size() - extStartInd);
			if (outType == ".ard")
			{
				exportType = 0;
			}
			else if (outType == ".bin")
			{
				exportType = 0;
			}
			else if (outType == ".dae" || outType == ".mdls" || outType == ".obj" || outType == ".wpn")
			{
				exportType = 0;
				// Check if any of the model transforms were enabled
				if (flipCheckBox->Checked || yCheckBox->Checked || normalCheckBox->Checked || combineCheckBox->Checked)
				{
					// Check if we need to flip UVs
					if (flipCheckBox->Checked)
					{
						successful = successful && flipUVCoordinates(*modelVertsG, *modelVertsG);
					}
					// Check if we need to rotate the model
					if (yCheckBox->Checked)
					{
						successful = successful && swapChirality(*modelJoints, *modelVertsG);
					}
					// Check if we need to normalize the model
					if (normalCheckBox->Checked)
					{
						successful = successful && normalizePosition(*modelVertsG, *modelVertsG, *modelJoints, modelJoints);
					}
					// Check if we need to combine the meshes
					if (combineCheckBox->Checked)
					{
						successful = successful && combineMeshes(*modelVertsR, *modelFaces);
					}
					// Convert the global verts to relative verts to apply all the transforms at once
					successful = successful && getVerticesRelative(*modelJoints, *modelVertsG, *modelVertsR);
				}
				if (successful)
				{
					if (outType == ".dae")
					{
						if (*assetType != ".dae")
						{
							// Convert to dae format
							*assetData = convertMeshToDae(*modelJoints, *modelVertsG, *modelFaces, std::vector<Animation>(), *assetName);
						}
					}
					else if (outType == ".mdls")
					{
						if (*assetType != ".mdls")
						{
							if (*templateData != "")
							{
								// Convert to mdls format
								*assetData = convertMeshToMdls(*modelJoints, *modelVertsR, *modelFaces, *templateData, "");
							}
							else
							{
								MessageBox::Show(this, L"Converting to MDLS Requires a Template", L"HyperCrown - Export",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								successful = false;
							}
						}
						if (successful)
						{
							// Set the textures
							*assetData = setMdlsTextures(*assetData, *assetTextures);
						}
					}
					else if (outType == ".moa")
					{
						if (*assetType != ".moa")
						{
							if (*templateData != "")
							{
								*assetData = convertMeshToMoa(*modelJoints, *modelVertsR, *modelFaces, *templateData, "");
							}
							else
							{
								MessageBox::Show(this, L"Converting to MOA Requires a Template", L"HyperCrown - Export",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								successful = false;
							}
						}
						if (successful)
						{
							*assetData = setMoaTextures(*assetData, *assetTextures);
						}
					}
					else if (outType == ".obj")
					{
						if (*assetType != ".obj")
						{
							// Convert to dae format
							*assetData = convertMeshToObj(*modelVertsG, *modelFaces);
							if (!binaryFileWrite(outName.substr(0, extStartInd) + ".mtl", createMTL(assetTextures->size(), *assetName)))
							{
								MessageBox::Show(this, L"Failed to Write to File", L"HyperCrown - Export",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								successful = false;
							}
						}
					}
					else // WPN
					{
						if (*assetType != ".wpn")
						{
							if (*templateData != "")
							{
								// Convert to wpn format
								*assetData = convertMeshToWpn(*modelVertsR, *modelFaces, *templateData);
							}
							else
							{
								MessageBox::Show(this, L"Converting to WPN Requires a Template", L"HyperCrown - Export",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								successful = false;
							}
						}
						if (successful)
						{
							// Set the textures
							*assetData = setWpnTextures(*assetData, *assetTextures);
						}
					}
				}
				else
				{
					MessageBox::Show(this, L"Failed to Transform Vertices", L"HyperCrown - Export",
						MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					successful = false;
				}
			}
			else if (outType == ".dpx")
			{
				exportType = 0;
			}
			else if (outType == ".mag")
			{
				exportType = 0;
			}
			else if (outType == ".mset")
			{
				exportType = 1;
				*msetData = setMsetTextures(*msetData, *msetTextures, *assetTextures, *refInds);
			}
			else if (outType == ".ps2")
			{
				exportType = 0;
				*assetData = setPs2Textures(*assetData, *assetTextures);
			}
			else if (outType == ".rtb" || outType == ".tarc")
			{
				exportType = 0;
			}
			else if (outType == ".se")
			{
				exportType = 0;
			}
			else if (outType == ".spe")
			{
				exportType = 0;
				*assetData = setSpeTextures(*assetData, *assetTextures);
			}
			else if (outType == ".tzb")
			{
				exportType = 0;
				*assetData = setTezbTextures(*assetData, *assetTextures);
			}
			else if (outType == ".vsb")
			{
				exportType = 0;
			}
			else if (outType == ".vset")
			{
				exportType = 0;
			}
			else if (outType == ".wd")
			{
				exportType = 0;
			}
			else
			{
				MessageBox::Show(this, L"Unsupported Output Type", L"HyperCrown - Export",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
				successful = false;
			}

			/*
			exportProgressBar->Value = exportProgressBar->Minimum;
			exportProgressBar->Visible = true;
			for (int i = exportProgressBar->Minimum; i <= exportProgressBar->Maximum; i++)
			{
				exportProgressBar->Value = i;
				System::Threading::Thread::Sleep(100);
			}
			System::Threading::Thread::Sleep(200);
			*/
			if (successful)
			{
				// Write to the file
				if (exportType == 0)
				{
					if (binaryFileWrite(outName, *assetData))
					{
						importTextBox->Text = std2system(outName);
					}
					else
					{
						MessageBox::Show(this, L"Failed to Write to File", L"HyperCrown - Export",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						successful = false;
					}
				}
				else if (exportType == 1)
				{
					if (binaryFileWrite(outName, *msetData))
					{
						msetTextBox->Text = std2system(outName);
					}
					else
					{
						MessageBox::Show(this, L"Failed to Write to File", L"HyperCrown - Export",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						successful = false;
					}
				}
				else
				{
					MessageBox::Show(this, L"Unable to Export Asset", L"HyperCrown - Export",
						MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					successful = false;
				}
				if (successful)
				{
					MessageBox::Show(this, L"Export Successful", L"HyperCrown - Export",
						MessageBoxButtons::OK, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1);
				}
			}
			/*
			this->exportProgressBar->Visible = false;
			*/
		}
		return;
	}

	private: System::Void templateButton_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog fd;
		fd.Filter = L"Character Model (*.mdls)|*.mdls|Weapon Model (*.wpn)|*.wpn";
		fd.FilterIndex = 0;
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			std::string filename = formatInputStr(system2std(fd.FileName));
			if (binaryFileRead(filename, *templateData))
			{
				templateTextBox->Text = std2system(filename);
			}
			else
			{
				templateTextBox->Text = L"";
				*templateData = "";
				MessageBox::Show(this, L"Failed to Read Template", L"HyperCrown - Import Template",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
		return;
	}

	private: System::Void msetButton_Click(System::Object^  sender, System::EventArgs^  e) {
		bool success = true;
		if (*assetData != "")
		{
			OpenFileDialog fd;
			fd.Filter = L"Character Moveset (*.mset)|*.mset";
			fd.FilterIndex = 0;
			if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
			{
				std::string filename = formatInputStr(system2std(fd.FileName));
				// Clear data structures
				msetTextures->clear();
				msetTextureList->Images->Clear();
				msetTextureListView->Items->Clear();
				msetSubFileListView->Items->Clear();
				msetTextureListView->Enabled = false;
				msetSubFileListView->Enabled = false;
				msetSubFileListView->View = View::List;
				// Read from the file
				if (binaryFileRead(filename, *msetData))
				{
					std::stringstream ss;
					int extInd = filename.find_first_of('.'), nameStartInd = filename.find_last_of('/');
					if (nameStartInd == filename.size())
					{
						nameStartInd = 0;
					}
					else
					{
						nameStartInd++;
					}
					*msetName = filename.substr(nameStartInd, extInd - nameStartInd);
					msetTextBox->Text = std2system(filename);
					*msetBaseTextures = getMsetTextures(*msetData, *assetTextures);
					*msetTextures = *msetBaseTextures;
					*refInds = getMsetRefInds(*msetData);
					for (unsigned int i = 0; i < refInds->size(); i++)
					{
						if ((*refInds)[i] >= assetTextures->size())
						{
							(*refInds)[i] = 0;
						}
					}
					// For each texture...
					for (unsigned int i = 0; i < msetTextures->size(); i++)
					{
						// Get texture i
						Texture ti = (*msetTextures)[i];
						// Intialize the bitmap
						Bitmap ^bmi;
						std::string iStr;
						ss.clear();
						ss.str("");
						ss << i;
						ss >> iStr;
						bmi = textureToBitmap(ti);
						// Populate the texture list
						// If this is the first image, use its dims
						if (i == 0)
						{
							msetTextureList->ImageSize = Drawing::Size(ti.width, ti.height);
						}
						msetTextureList->Images->Add(bmi);
						// Create ListView Item
						ListViewItem ^lvi = gcnew ListViewItem(std2system(*msetName + "-m" + iStr), i);
						// Add the item to the ListView
						msetTextureListView->Items->Add(lvi);
					}
					if (msetTextureListView->Items->Count > 0)
					{
						msetTextureListView->Enabled = true;
					}
					if (msetSubFileListView->Items->Count > 0)
					{
						msetSubFileListView->View = View::Details;
						msetSubFileListView->Enabled = true;
					}
				}
				else
				{
					MessageBox::Show(this, L"Failed to Read MSET", L"HyperCrown - Import MSET",
						MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					success = false;
				}
			}
		}
		else
		{
			MessageBox::Show(this, L"Importing a MSET Requires an Imported Asset", L"HyperCrown - Import MSET",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			success = false;
		}
		if (!success)
		{
			msetTextBox->Text = L"";
			*msetData = "";
		}
		return;
	}

	private: System::Void msetTextureListView_Scroll(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		msetNumericUpDown->Visible = false;
		msetNumericUpDown->Enabled = false;
		focusedMsetItem = nullptr;
		return;
	}
	
	private: System::Void msetTextureListView_Leave(System::Object^ sender, System::EventArgs^ e) {
		System::Drawing::Point mp = msetTextureListView->MousePosition, lp = msetTextureListView->Location, wp = this->Location;
		// Determine bounds
		int xMin = msetTextureListView->Bounds.Left, xMax = msetTextureListView->Bounds.Right - 22, yMin = msetTextureListView->Bounds.Top + 3, yMax = msetTextureListView->Bounds.Bottom - 2;
		// Translate Mouse Location to ListView frame of reference
		mp.X -= (wp.X + 10);
		mp.Y -= (wp.Y + 30);
		// Check if mouse has left bounds
		if (mp.X < xMin || mp.X > xMax || mp.Y < yMin || mp.Y > yMax)
		{
			msetNumericUpDown->Visible = false;
			msetNumericUpDown->Enabled = false;
			focusedMsetItem = nullptr;
		}
		return;
	}

	private: System::Void msetTextureListView_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		System::Drawing::Point mp = msetTextureListView->MousePosition, lp = msetTextureListView->Location, wp = this->Location;
		System::Collections::IEnumerator ^it = msetTextureListView->Items->GetEnumerator();
		unsigned int i = 0;
		// Translate Mouse Location to ListView frame of reference
		mp.X -= (wp.X + lp.X + 10);
		mp.Y -= (wp.Y + lp.Y + 30);
		// Check each item
		while (it->MoveNext())
		{
			ListViewItem ^lvi = (ListViewItem ^)it->Current;
			// Determine bounds
			int yMin = lvi->Bounds.Top - 4, yMax = lvi->Bounds.Bottom + 4;
			// Check if mouse has left bounds
			if (mp.Y >= yMin && mp.Y <= yMax)
			{
				System::Drawing::Point lvl = msetTextureListView->Location;
				System::Drawing::Size lvs = msetTextureListView->Size;
				int x = lvl.X + lvi->Bounds.Right - 16, y = lvl.Y + lvi->Bounds.Top + 8, w = 40, h = 20;
				unsigned int ind = lvi->Index;
				if (lvl.X <= x && lvl.Y <= y && (lvl.X + lvs.Width) >= (x + w) && (lvl.Y + lvs.Height) >= (y + h))
				{
					focusedMsetItem = lvi;
					msetNumericUpDown->SetBounds(x, y, w, h);
					msetNumericUpDown->Value = (*refInds)[ind];
					msetNumericUpDown->Maximum = assetTextures->size() - 1;
					msetNumericUpDown->Enabled = true;
					msetNumericUpDown->Visible = true;
				}
				else
				{
					msetNumericUpDown->Visible = false;
					msetNumericUpDown->Enabled = false;
					focusedMsetItem = nullptr;
				}
				break;
			}
			else
			{
				i++;
			}
		}
		if (i == msetTextures->size())
		{
			// Disable the Numeric Up Down
			msetNumericUpDown->Visible = false;
			msetNumericUpDown->Enabled = false;
			focusedMsetItem = nullptr;
		}
		return;
	}

	private: System::Void msetNumericUpDown_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
		if (focusedMsetItem != nullptr)
		{
			unsigned int ind = focusedMsetItem->Index, val = (unsigned int)msetNumericUpDown->Value;
			if (val < assetTextures->size() && val != (*refInds)[ind])
			{
				(*refInds)[ind] = val;
				msetTextureList->Images->Clear();
				for (unsigned int i = 0; i < msetTextures->size(); i++)
				{
					Texture t, tr = (*assetTextures)[(*refInds)[i]];
					// Intialize the bitmap
					Bitmap ^bmi;
					// Check if we need to update the palette
					if (i == ind)
					{
						t = (*msetBaseTextures)[i];
						t = associateTexture(t, tr);
						// Replace the mset image
						(*msetTextures)[i] = t;
					}
					else
					{
						t = (*msetTextures)[i];
					}
					// Convert the texture to a bitmap
					bmi = textureToBitmap(t);
					// Add the texture to the list
					msetTextureList->Images->Add(bmi);
				}
			}
		}
		return;
	}

	private: System::Void importAssetImageToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog fd;
		fd.Filter = L"PNG (*.png)|*.png";
		fd.FilterIndex = 0;
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			Texture t;
			std::stringstream ss;
			if (readTexture(system2std(fd.FileName), t))
			{
				Collections::IEnumerator ^it = assetTextureListView->Items->GetEnumerator();
				while (it->MoveNext())
				{
					if (((ListViewItem ^)it->Current)->Selected)
					{
						unsigned int ind = ((ListViewItem ^)it->Current)->Index;
						Texture prevT = (*assetTextures)[ind];
						t.indexWidth = prevT.indexWidth;
						if (prevT.palette.colors.size() > 1)
						{
							unsigned int numPalettes = prevT.palette.colors.size();
							Palette p;
							for (unsigned int i = 0; i < t.height; i++)
							{
								for (unsigned int j = 0; j < t.width; j++)
								{
									if (i < prevT.height && j < prevT.width)
									{
										unsigned int index = i * t.width + j, prevIndex = i * prevT.width + j;
										t.paletteIndices[index] = prevT.paletteIndices[prevIndex];
									}
								}
							}
							p.colors.reserve(numPalettes);
							for (unsigned int i = 0; i < numPalettes; i++)
							{
								p.colors.push_back(std::vector<KHColor>());
							}
							for (unsigned int i = 0; i < t.indices.size(); i++)
							{
								KHColor ci = t.palette.colors.back()[t.indices[i]];
								unsigned int j, paletteIndex = t.paletteIndices[i];
								for (j = 0; j < p.colors[paletteIndex].size(); j++)
								{
									KHColor cp = p.colors[paletteIndex][j];
									if (ci == cp)
									{
										t.indices[i] = j;
										break;
									}
								}
								if (j == p.colors[paletteIndex].size())
								{
									t.indices[i] = j;
									p.colors[t.paletteIndices[i]].push_back(ci);
								}
							}
							for (unsigned int i = 0; i < numPalettes; i++)
							{
								for (unsigned int j = p.colors[i].size(); j < 256; j++)
								{
									KHColor c;
									c.r = 0;
									c.g = 0;
									c.b = 0;
									c.a = 0;
									p.colors[i].push_back(c);
								}
							}
							t.palette = p;
						}
						(*assetTextures)[ind] = t;
						assetTextureList->Images->Clear();
						for (unsigned int i = 0; i < assetTextures->size(); i++)
						{
							// Intialize the bitmap
							Bitmap ^bmi = textureToBitmap((*assetTextures)[i]);
							assetTextureList->Images->Add(bmi);
						}
						// Check if we need to update mset textures
						if (msetTextures->size() > 0)
						{
							// For each mset texture
							msetTextureList->Images->Clear();
							for (unsigned int i = 0; i < msetTextures->size(); i++)
							{
								// Store the texture
								Texture mt;
								// Intialize the bitmap
								Bitmap ^bmi;
								if (ind == (*refInds)[i])
								{
									mt = (*msetBaseTextures)[i];
									mt = associateTexture(mt, t);
									(*msetTextures)[i] = mt;
								}
								else
								{
									mt = (*msetTextures)[i];
								}
								bmi = textureToBitmap(mt);
								// Replace the mset image
								msetTextureList->Images->Add(bmi);
							}
						}
						break;
					}
				}
			}
			else
			{
				MessageBox::Show(this, L"Failed to Read Image File", L"HyperCrown - Import Texture",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
		return;
	}

	private: System::Void exportAssetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		Collections::IEnumerator ^it = assetTextureListView->Items->GetEnumerator();
		std::string path = system2std(importTextBox->Text);
		unsigned int ind = path.find_last_of('/') + 1;
		if (ind <= path.size())
		{
			path = path.substr(0, ind);
		}
		while(it->MoveNext())
		{
			if (((ListViewItem ^)it->Current)->Selected)
			{
				writeTexture((*assetTextures)[((ListViewItem ^)it->Current)->Index], path + system2std(((ListViewItem ^)it->Current)->Text) + ".png");
			}
		}
		return;
	}

	private: System::Void exportAllAssetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		Collections::IEnumerator ^it = assetTextureListView->Items->GetEnumerator();
		std::string path = system2std(importTextBox->Text);
		unsigned int ind = path.find_last_of('/') + 1;
		if (ind <= path.size())
		{
			path = path.substr(0, ind);
		}
		while (it->MoveNext())
		{
			writeTexture((*assetTextures)[((ListViewItem ^)it->Current)->Index], path + system2std(((ListViewItem ^)it->Current)->Text) + ".png");
		}
		return;
	}

	private: System::Void addAssetImageToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		if (*assetData != "")
		{
			OpenFileDialog fd;
			fd.Filter = L"PNG (*.png)|*.png";
			fd.FilterIndex = 0;
			if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
			{
				Texture t;
				if (readTexture(system2std(fd.FileName), t))
				{
					std::stringstream ss;
					unsigned int ind = assetTextures->size();
					std::string indStr;
					ss << ind;
					ss >> indStr;
					// Add the texture to textures vector
					assetTextures->push_back(t);
					// Intialize the bitmap
					Bitmap ^bmi = textureToBitmap(t);
					assetTextureList->Images->Add(bmi);
					assetTextureListView->Items->Add(gcnew ListViewItem(std2system(*assetName + "-" + indStr), ind));
				}
				else
				{
					MessageBox::Show(this, L"Failed to Read Image File", L"HyperCrown - Add Image",
						MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
				}
			}
		}
		else
		{
			MessageBox::Show(this, L"Import an Asset First", L"HyperCrown - Add Image",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: System::Void removeAssetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		if (*assetData != "")
		{
			std::vector<Texture> updatedTextureVector;
			std::stringstream ss;
			IEnumerator^ enm = assetTextureListView->Items->GetEnumerator(), ^enm2;
			unsigned int texInd = 0, numSavedTextures, numTextures = assetTextures->size(), numDeleted = 0;
			updatedTextureVector.reserve(assetTextures->size());
			// Copy in the items we are not deleting
			while (enm->MoveNext())
			{
				ListViewItem^ itm = (ListViewItem^)enm->Current;
				numSavedTextures = updatedTextureVector.size();
				if (itm->Selected)
				{
					enm2 = assetTextureListView->Items->GetEnumerator();
					for (int i = 0; i <= itm->Index; i++)
					{
						enm2->MoveNext();
					}
					while (enm2->MoveNext())
					{
						((ListViewItem^)enm2->Current)->ImageIndex--;
					}
					if (msetTextures->size() > 0)
					{
						bool redraw = false;
						for (unsigned int i = 0; i < refInds->size(); i++)
						{
							unsigned int rInd = (*refInds)[i];
							if (rInd >= texInd)
							{
								if (rInd > texInd)
								{
									(*refInds)[i]--;
								}
								else
								{
									Texture rt, t = (*msetBaseTextures)[i];
									if (updatedTextureVector.size() > 0)
									{
										(*refInds)[i] = updatedTextureVector.size() - 1;
										rt = updatedTextureVector[(*refInds)[i]];
									}
									else if (texInd + 1 < numTextures)
									{
										(*refInds)[i] = texInd + 1;
										rt = (*assetTextures)[(*refInds)[i]];
									}
									else
									{
										msetBaseTextures->clear();
										msetTextures->clear();
										refInds->clear();
										msetTextureList->Images->Clear();
										msetTextureListView->Items->Clear();
										break;
									}
									t = associateTexture(t, rt);
									(*msetTextures)[i] = t;
									redraw = true;
								}
							}
						}
						if (redraw)
						{
							msetTextureList->Images->Clear();
							for (unsigned int i = 0; i < msetTextures->size(); i++)
							{
								// Intialize the bitmap
								Bitmap ^bmi = textureToBitmap((*msetTextures)[i]);
								msetTextureList->Images->Add(bmi);
							}
						}
					}
					assetTextureList->Images->RemoveAt(itm->Index);
					assetTextureListView->Items->RemoveAt(itm->Index);
				}
				else
				{
					std::string indStr;
					ss.clear();
					ss.str("");
					ss << numSavedTextures;
					ss >> indStr;
					((ListViewItem^)enm->Current)->Text = std2system(*assetName + "-" + indStr);
					updatedTextureVector.push_back((*assetTextures)[texInd]);
				}
				texInd++;
			}
			// Update the asset textures
			*assetTextures = updatedTextureVector;
		}
		else
		{
			MessageBox::Show(this, L"Import an Asset First", L"HyperCrown - Remove Image",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: System::Void addMsetImageToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		if (*assetData != "" && *msetData != "")
		{
			if (assetTextures->size() > 0)
			{
				OpenFileDialog fd;
				fd.Filter = L"PNG (*.png)|*.png";
				fd.FilterIndex = 0;
				if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
				{
					Texture t;
					if (readTexture(system2std(fd.FileName), t))
					{
						std::stringstream ss;
						unsigned int ind = msetTextures->size();
						std::string indStr;
						ss << ind;
						ss >> indStr;
						// Add the texture to textures vector
						msetBaseTextures->push_back(t);
						refInds->push_back(0);
						t = associateTexture(t, (*assetTextures)[refInds->back()]);
						msetTextures->push_back(t);
						// Intialize the bitmap
						Bitmap ^bmi = textureToBitmap(t);
						msetTextureList->Images->Add(bmi);
						msetTextureListView->Items->Add(gcnew ListViewItem(std2system(*assetName + "-m" + indStr), ind));
					}
					else
					{
						MessageBox::Show(this, L"Failed to Read Image File", L"HyperCrown - Add Image",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
					}
				}
			}
			else
			{
				MessageBox::Show(this, L"Import an Asset with a Texture First", L"HyperCrown - Add Image",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
		else
		{
			MessageBox::Show(this, L"Import an Asset First", L"HyperCrown - Add Image",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: System::Void removeMsetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		if (*msetData != "")
		{
			std::vector<Texture> updatedTextureVector, updatedBaseTextureVector;
			std::vector<unsigned int> updatedRefInds;
			std::stringstream ss;
			IEnumerator^ enm = msetTextureListView->Items->GetEnumerator(), ^enm2;
			unsigned int ind;
			updatedTextureVector.reserve(msetTextures->size());
			updatedBaseTextureVector.reserve(msetBaseTextures->size());
			updatedRefInds.reserve(refInds->size());
			// Copy in the items we are not deleting
			while (enm->MoveNext())
			{
				ListViewItem^ itm = (ListViewItem^)enm->Current;
				ind = updatedTextureVector.size();
				if (itm->Selected)
				{
					enm2 = msetTextureListView->Items->GetEnumerator();
					for (int i = 0; i <= itm->Index; i++)
					{
						enm2->MoveNext();
					}
					while (enm2->MoveNext())
					{
						((ListViewItem^)enm2->Current)->ImageIndex--;
					}
					msetTextureList->Images->RemoveAt(itm->Index);
					msetTextureListView->Items->RemoveAt(itm->Index);
				}
				else
				{
					std::string indStr;
					ss.clear();
					ss.str("");
					ss << ind;
					ss >> indStr;
					((ListViewItem^)enm->Current)->Text = std2system(*msetName + "-m" + indStr);
					updatedTextureVector.push_back((*msetTextures)[itm->Index]);
					updatedBaseTextureVector.push_back((*msetBaseTextures)[itm->Index]);
					updatedRefInds.push_back((*refInds)[itm->Index]);
				}
			}
			// Update the asset textures
			*msetTextures = updatedTextureVector;
			*msetBaseTextures = updatedBaseTextureVector;
			*refInds = updatedRefInds;
		}
		else
		{
			MessageBox::Show(this, L"Import an MSET First", L"HyperCrown - Remove Image",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: System::Void exportMsetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		Collections::IEnumerator ^it = msetTextureListView->Items->GetEnumerator();
		std::string path = system2std(msetTextBox->Text);
		unsigned int ind = path.find_last_of('/') + 1;
		if (ind <= path.size())
		{
			path = path.substr(0, ind);
		}
		while (it->MoveNext())
		{
			if (((ListViewItem ^)it->Current)->Selected)
			{
				writeTexture((*msetTextures)[((ListViewItem ^)it->Current)->Index], path + system2std(((ListViewItem ^)it->Current)->Text) + ".png");
			}
		}
		return;
	}

	private: System::Void exportAllMsetImagesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		Collections::IEnumerator ^it = msetTextureListView->Items->GetEnumerator();
		std::string path = system2std(msetTextBox->Text);
		unsigned int ind = path.find_last_of('/') + 1;
		if (ind <= path.size())
		{
			path = path.substr(0, ind);
		}
		while (it->MoveNext())
		{
			writeTexture((*msetTextures)[((ListViewItem ^)it->Current)->Index], path + system2std(((ListViewItem ^)it->Current)->Text) + ".png");
		}
		return;
	}

	private: System::Void importMsetImageToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog fd;
		fd.Filter = L"PNG (*.png)|*.png";
		fd.FilterIndex = 0;
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			Texture t;
			std::stringstream ss;
			if (readTexture(system2std(fd.FileName), t))
			{
				Collections::IEnumerator ^it = msetTextureListView->Items->GetEnumerator();
				while (it->MoveNext())
				{
					if (((ListViewItem ^)it->Current)->Selected)
					{
						unsigned int ind = ((ListViewItem ^)it->Current)->Index, i = 0, rInd = (*refInds)[ind];
						if (rInd < assetTextures->size())
						{
							Texture rt = (*assetTextures)[rInd];
							(*msetBaseTextures)[ind] = t;
							t = associateTexture(t, rt);
							(*msetTextures)[ind] = t;
							msetTextureList->Images->Clear();
							for (unsigned int i = 0; i < msetTextures->size(); i++)
							{
								// Intialize the bitmap
								Bitmap ^bmi = textureToBitmap((*msetTextures)[i]);
								msetTextureList->Images->Add(bmi);
							}
						}
						break;
					}
				}
			}
			else
			{
				MessageBox::Show(this, L"Failed to Read Image File", L"HyperCrown - Import Texture",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
		return;
	}

	private: System::Void assignPaletteToPixelsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		Collections::IEnumerator ^it = assetTextureListView->Items->GetEnumerator();
		while (it->MoveNext())
		{
			ListViewItem ^ lvi = (ListViewItem ^)it->Current;
			if (lvi->Selected)
			{
				unsigned int ind = lvi->Index;
				Texture t = (*assetTextures)[ind];
				// Create Palette Assignment form and show it
				PaletteAssignment ^pa = gcnew PaletteAssignment(&t, lvi->Text);
				pa->ShowDialog();
				// Check if the form updated the texture
				if (pa->DialogResult != System::Windows::Forms::DialogResult::Cancel)
				{
					// Save the updated texture
					(*assetTextures)[ind] = t;
					assetTextureList->Images->Clear();
					// Update the asset texture ListView
					for (unsigned int i= 0; i < assetTextures->size(); i++)
					{
						Bitmap ^bmi = textureToBitmap((*assetTextures)[i]);
						assetTextureList->Images->Add(bmi);
					}
				}
				// Exit the loop
				break;
			}
		}
	}

	private: System::Void importAssetSubFileToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator ^enm = assetSubFileListView->Items->GetEnumerator();
		while (enm->MoveNext())
		{
			if (((ListViewItem^)enm->Current)->Selected)
			{
				importAssetSubFile(((ListViewItem^)enm->Current)->Index);
				if ((*assetSubfileStatus)[((ListViewItem^)enm->Current)->Index])
				{
					((ListViewItem^)enm->Current)->BackColor = Color::White;
					((ListViewItem^)enm->Current)->SubItems->RemoveAt(2);
					((ListViewItem^)enm->Current)->SubItems->Add(L"True");
				}
				else
				{
					((ListViewItem^)enm->Current)->BackColor = Color::LightGray;
					((ListViewItem^)enm->Current)->SubItems->RemoveAt(2);
					((ListViewItem^)enm->Current)->SubItems->Add(L"False");
				}
				break;
			}
		}
		return;
	}

	private: System::Void exportAssetSubFilesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator ^enm = assetSubFileListView->Items->GetEnumerator();
		FolderBrowserDialog fbd;
		int ind = importTextBox->Text->LastIndexOf(L"/");
		if (ind >= 0)
		{
			fbd.SelectedPath = importTextBox->Text->Substring(0, ind)->Replace(L"/", L"\\");
		}
		SendKeys::Send("{TAB}{TAB}{RIGHT}");
		if (fbd.ShowDialog() == Windows::Forms::DialogResult::OK)
		{
			while (enm->MoveNext())
			{
				if (((ListViewItem^)enm->Current)->Selected)
				{
					exportAssetSubFile(((ListViewItem^)enm->Current)->Index, system2std(fbd.SelectedPath));
				}
			}
		}
		return;
	}

	private: System::Void exportAssetAllToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		FolderBrowserDialog fbd;
		int ind = importTextBox->Text->LastIndexOf(L"/");
		if (ind >= 0)
		{
			fbd.SelectedPath = importTextBox->Text->Substring(0, ind)->Replace(L"/", L"\\");
		}
		SendKeys::Send("{TAB}{TAB}{RIGHT}");
		if (fbd.ShowDialog() == Windows::Forms::DialogResult::OK)
		{
			for (unsigned int i = 0; i < assetSubfiles->size(); i++)
			{
				if ((*assetSubfileStatus)[i])
				{
					exportAssetSubFile(i, system2std(fbd.SelectedPath));
				}
			}
		}
		return;
	}

	private: System::Void subFileListView_ColumnWidthChanging(System::Object^ sender, ColumnWidthChangingEventArgs^ e) {
		e->Cancel = true;
		IEnumerator^ enm = assetSubFileListView->Columns->GetEnumerator();
		for (int i = 0; i <= e->ColumnIndex; i++)
		{
			enm->MoveNext();
		}
		e->NewWidth =  ((ColumnHeader^)enm->Current)->Width;
		return;
	}

	private: System::Void importMsetSubFileToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator ^enm = msetSubFileListView->Items->GetEnumerator();
		while (enm->MoveNext())
		{
			if (((ListViewItem^)enm->Current)->Selected)
			{
				importMsetSubFile(((ListViewItem^)enm->Current)->Index);
				break;
			}
		}
		return;
	}

	private: System::Void exportMsetSubFileToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator ^enm = msetSubFileListView->Items->GetEnumerator();
		FolderBrowserDialog fbd;
		int ind = importTextBox->Text->LastIndexOf(L"/");
		if (ind >= 0)
		{
			fbd.SelectedPath = importTextBox->Text->Substring(0, ind)->Replace(L"/", L"\\");
		}
		SendKeys::Send("{TAB}{TAB}{RIGHT}");
		if (fbd.ShowDialog() == Windows::Forms::DialogResult::OK)
		{
			while (enm->MoveNext())
			{
				if (((ListViewItem^)enm->Current)->Selected)
				{
					exportMsetSubFile(((ListViewItem^)enm->Current)->Index, system2std(fbd.SelectedPath));
				}
			}
		}
		return;
	}

	private: System::Void exportMsetAllToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		IEnumerator ^enm = msetSubFileListView->Items->GetEnumerator();
		FolderBrowserDialog fbd;
		int ind = importTextBox->Text->LastIndexOf(L"/");
		if (ind >= 0)
		{
			fbd.SelectedPath = importTextBox->Text->Substring(0, ind)->Replace(L"/", L"\\");
		}
		SendKeys::Send("{TAB}{TAB}{RIGHT}");
		if (fbd.ShowDialog() == Windows::Forms::DialogResult::OK)
		{
			while (enm->MoveNext())
			{
				exportMsetSubFile(((ListViewItem^)enm->Current)->Index, system2std(fbd.SelectedPath));
			}
		}
		return;
	}

	private: void exportAssetSubFile(unsigned int i, std::string path)
	{
		bool success = true;
		if (path != "" && path.back() != '\\')
		{
			path += "\\";
		}
		if (i < assetSubfiles->size())
		{
			if ((*assetSubfileStatus)[i])
			{
				std::stringstream ss;
				std::string sft = (*assetSubfileTypes)[i], iStr, ext = "";
				std::vector<std::string> files;
				std::vector<Texture> images;
				ss << i;
				ss >> iStr;
				if (sft == "FILEs")
				{
					if (*assetType == ".rtb" || *assetType == ".tarc")
					{
						files = getTarcFiles(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export FILEs from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string jStr, magic = "";
						if (files[j].size() >= 4)
						{
							magic = files[j].substr(0, 4);
						}
						if (magic == "MOPB")
						{
							ext = ".mlb";
						}
						else if (magic == "TARC")
						{
							ext = ".tarc";
						}
						else if (magic == "TEZB")
						{
							ext = ".tzb";
						}
						else
						{
							ext = ".bin";
						}
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						binaryFileWrite(path + *assetName + "-s" + iStr + "-" + jStr + ext, files[j]);
					}
				}
				else if (sft == "MFAs")
				{
					std::vector<std::string> filenames;
					unsigned int fnInd = 0;
					ext = ".mfa";
					if (*assetType == ".ard")
					{
						files = getArdMfas(*assetData);
						filenames = getArdFilenames(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export MFAs from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string jStr, fn;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						fn = *assetName + "-s" + iStr + "-" + jStr;
						for (unsigned int k = fnInd; k < filenames.size(); k++, fnInd++)
						{
							std::string tmp = filenames[k], tmpExt;
							unsigned int dotInd = tmp.find_last_of('.');
							if (dotInd <= tmp.size())
							{
								tmpExt = tmp.substr(dotInd);
								if (tmpExt == ".mfa")
								{
									fn = tmp;
									ext = "";
									fnInd++;
									break;
								}
							}
						}
						binaryFileWrite(path + fn + ext, files[j]);
					}
				}
				else if (sft == "MOAs")
				{
					std::vector<std::string> filenames;
					unsigned int fnInd = 0;
					ext = ".moa";
					if (*assetType == ".ard")
					{
						files = getArdMoas(*assetData);
						filenames = getArdFilenames(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export MOAs from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string jStr, fn;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						fn = *assetName + "-s" + iStr + "-" + jStr;
						for (unsigned int k = fnInd; k < filenames.size(); k++, fnInd++)
						{
							std::string tmp = filenames[k], tmpExt;
							unsigned int dotInd = tmp.find_last_of('.');
							if (dotInd <= tmp.size())
							{
								tmpExt = tmp.substr(dotInd);
								if (tmpExt == ".moa")
								{
									fn = tmp;
									ext = "";
									fnInd++;
									break;
								}
							}
						}
						binaryFileWrite(path + fn + ext, files[j]);
					}
				}
				else if (sft == "TIM2")
				{
					ext = ".png";
					if (*assetType == ".mdls")
					{
						images = getMdlsHUDTextures(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export TIM2 from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < images.size(); j++)
					{
						std::string jStr;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						writeTexture(images[j], path + *assetName + "-s" + iStr + "-" + jStr + ext);
					}
				}
				else if (sft == "SPEs")
				{
					ext = ".spe";
					if (*assetType == ".dpx")
					{
						files = getDpxSpecialEffects(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export SPEs from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string jStr;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						binaryFileWrite(path + *assetName + "-s" + iStr + "-" + jStr + ext, files[j]);
					}
				}
				else if (sft == "VAGs")
				{
					ext = ".vag";
					if (*assetType == ".mdls")
					{
						files = getMdlsVags(*assetData);
					}
					else if (*assetType == ".vsb")
					{
						files = getVsbVags(*assetData);
					}
					else if (*assetType == ".vset")
					{
						files = getVsetVags(*assetData);
					}
					else if (*assetType == ".wd")
					{
						files = getWdVags(*assetData, *assetName);
					}
					else
					{
						MessageBox::Show(this, L"Unable to export VAGs from this asset type", L"HyperCrown - Export Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string jStr, fn;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						fn = *assetName + "-s" + iStr + "-" + jStr;
						if (files[j].size() >= 4)
						{
							std::string magic = files[j].substr(0, 4);
							if (magic == "VAGp")
							{
								if (files[j].size() >= 48)
								{
									unsigned int dotInd;
									fn = std::string(files[j].substr(32, 16).c_str());
									dotInd = (unsigned int)fn.find_last_of('.');
									if (dotInd < fn.size() && fn.substr(dotInd, 4) == ".vag")
									{
										ext = "";
									}
								}
							}
						}
						binaryFileWrite(path + fn + ext, files[j]);
					}
				}
				else
				{
					if (sft == "DPX")
					{
						ext = ".dpx";
					}
					else if (sft == "SE")
					{
						ext = ".se";
					}
					else if (sft == "WD")
					{
						ext = ".wd";
					}
					else
					{
						ext = ".bin";
					}
					binaryFileWrite(path + *assetName + "-s" + iStr + ext, (*assetSubfiles)[i]);
				}
			}
			else
			{
				MessageBox::Show(this, L"Nothing to Export", L"HyperCrown - Export Sub-File",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
				success = false;
			}

		}
		else
		{
			MessageBox::Show(this, L"Invalid Subfile index", L"HyperCrown - Export Sub-File",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			success = false;
		}
		if (success)
		{
			MessageBox::Show(this, L"Export Successful", L"HyperCrown - Export Sub-File",
				MessageBoxButtons::OK, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: void importAssetSubFile(unsigned int i)
	{
		bool success = true;
		if (i < assetSubfiles->size())
		{
			std::string sft = (*assetSubfileTypes)[i];
			if (sft == "FILEs" || sft == "MFAs" || sft == "MOAs" || sft == "SPEs" || sft == "VAGs")
			{
				// Prompt User for Multiple Files
				std::vector<std::string> files, filenames, fileTypes;
				std::stringstream ss;
				std::string filter = "", iStr;
				MultiSubFileImport^ msfi;
				ss << i;
				ss >> iStr;
				if (sft == "FILEs")
				{
					if (*assetType == ".rtb" || *assetType == ".tarc")
					{
						files = getTarcFiles(*assetData);
						filter = "MLB (*.mlb)|*.mlb|TARC (*.tarc)|*.tarc|TEZB (*.tzb)|*.tzb|Unknown Binary (*.bin)|*.bin";
					}
					else
					{
						MessageBox::Show(this, L"Unable to import FILEs to this asset type", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					fileTypes.reserve(files.size());
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string type = "????";
						if (files[j].size() >= 4)
						{
							std::string magic = files[j].substr(0, 4);
							if (magic == "MOPB")
							{
								type = "MLB";
							}
							else if (magic == "TARC")
							{
								type = "TARC";
							}
							else if (magic == "TEZB")
							{
								type = "TZB";
							}
						}
						fileTypes.push_back(type);
					}
				}
				else if (sft == "MFAs")
				{
					std::vector<std::string> tmpFilenames;
					unsigned int tmpInd = 0;
					filter = "ARD MFA (*.mfa)|*.mfa";
					if (*assetType == ".ard")
					{
						files = getArdMfas(*assetData);
						tmpFilenames = getArdFilenames(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to import MFAs to this asset type", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					filenames.reserve(files.size());
					fileTypes.reserve(files.size());
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string fn = *assetName + "-s" + iStr + "-", jStr;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						fn += jStr;
						for (unsigned int k = tmpInd; k < tmpFilenames.size(); k++, tmpInd++)
						{
							std::string tfn = tmpFilenames[k];
							unsigned int dotInd = tfn.find_last_of('.');
							if (dotInd < tfn.size())
							{
								if (tfn.substr(dotInd) == ".mfa")
								{
									fn = tfn.substr(0, dotInd);
									tmpInd++;
									break;
								}
							}
						}
						filenames.push_back(fn);
						fileTypes.push_back("MFA");
					}
				}
				else if (sft == "MOAs")
				{
					std::vector<std::string> tmpFilenames;
					unsigned int tmpInd = 0;
					filter = "ARD MOA (*.moa)|*.moa";
					if (*assetType == ".ard")
					{
						files = getArdMoas(*assetData);
						tmpFilenames = getArdFilenames(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to import MOAs to this asset type", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					filenames.reserve(files.size());
					fileTypes.reserve(files.size());
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string fn = *assetName + "-s" + iStr + "-", jStr;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						fn += jStr;
						for (unsigned int k = tmpInd; k < tmpFilenames.size(); k++, tmpInd++)
						{
							std::string tfn = tmpFilenames[k];
							unsigned int dotInd = tfn.find_last_of('.');
							if (dotInd < tfn.size())
							{
								if (tfn.substr(dotInd) == ".moa")
								{
									fn = tfn.substr(0, dotInd);
									tmpInd++;
									break;
								}
							}
						}
						filenames.push_back(fn);
						fileTypes.push_back("MOA");
					}
				}
				else if (sft == "SPEs")
				{
					filter = "SPE (*.spe)|*.spe";
					if (*assetType == ".dpx")
					{
						files = getDpxSpecialEffects(*assetData);
					}
					else
					{
						MessageBox::Show(this, L"Unable to import SPEs to this asset type", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					fileTypes.reserve(files.size());
					for (unsigned int j = 0; j < files.size(); j++)
					{
						fileTypes.push_back("SPE");
					}
				}
				else // if (sft == "VAGs")
				{
					filter = "VAG (*.vag)|*.vag";
					if (*assetType == ".mdls")
					{
						files = getMdlsVags(*assetData);
					}
					else if (*assetType == ".vsb")
					{
						files = getVsbVags(*assetData);
					}
					else if (*assetType == ".vset")
					{
						files = getVsetVags(*assetData);
					}
					else if (*assetType == ".wd")
					{
						files = getWdVags(*assetData, *assetName);
					}
					else
					{
						MessageBox::Show(this, L"Unable to import VAGs to this asset type", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					filenames.reserve(files.size());
					fileTypes.reserve(files.size());
					for (unsigned int j = 0; j < files.size(); j++)
					{
						std::string name = *assetName + "-s" + iStr + "-", jStr;
						ss.clear();
						ss.str("");
						ss << j;
						ss >> jStr;
						name += jStr;
						if (files[j].size() >= 48)
						{
							std::string magic = files[j].substr(0, 4);
							if (magic == "VAGp")
							{
								name = std::string(files[j].substr(32, 16).c_str());
							}
						}
						filenames.push_back(name);
						fileTypes.push_back("VAG");
					}
				}
				if (success)
				{
					msfi = gcnew MultiSubFileImport(&files, filenames, fileTypes, std2system(*assetName + "-s" + iStr), std2system(filter));
					msfi->ShowDialog();
					if (msfi->DialogResult != Windows::Forms::DialogResult::Cancel)
					{
						if (sft == "FILEs")
						{
							// if (*assetType == ".rtb" || *assetType == ".tarc")
							{
								*assetData = setTarcFiles(*assetData, files);
							}
						}
						else if (sft == "MFAs")
						{
							// if (*assetType == ".ard")
							{
								*assetData = setArdMfas(*assetData, files);
							}
						}
						else if (sft == "MOAs")
						{
							// if (*assetType == ".ard")
							{
								*assetData = setArdMoas(*assetData, files);
							}
						}
						else if (sft == "SPEs")
						{
							// if (*assetType == ".dpx")
							{
								*assetData = setDpxSpecialEffects(files);
							}
						}
						else // if (sft == "VAGs")
						{
							if (*assetType == ".mdls")
							{
								*assetData = setMdlsVags(*assetData, files);
							}
							else if (*assetType == ".vsb")
							{
								*assetData = setVsbVags(*assetData, files);
							}
							else if (*assetType == ".vset")
							{
								*assetData = setVsetVags(files);
							}
							else // if (*assetType == ".wd")
							{
								*assetData = setWdVags(*assetData, files);
							}
						}
						(*assetSubfileStatus)[i] = files.size() > 0;
					}
					else
					{
						success = false;
					}
				}
			}
			else if (sft == "TIM2")
			{
				// Prompt User for Single File (Texture)
				Texture t;
				OpenFileDialog fd;
				fd.Filter = L"PNG (*.png)|*.png";
				fd.FilterIndex = 0;
				fd.AddExtension = true;
				if (fd.ShowDialog() == Windows::Forms::DialogResult::OK)
				{
					if (readTexture(system2std(fd.FileName), t))
					{
						if (*assetType == ".mdls")
						{
							*assetData = setMdlsHUDTexture(*assetData, t);
						}
						else
						{
							MessageBox::Show(this, L"Unable to import TIM2 to this asset type", L"HyperCrown - Import Sub-File",
								MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
							success = false;
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to read texture from file system", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					(*assetSubfileStatus)[i] = true;
				}
				else
				{
					success = false;
				}
			}
			else if (sft == "DPX" || sft == "SE" || sft == "WD")
			{
				// Prompt User for Single File
				std::string file;
				OpenFileDialog fd;
				fd.FilterIndex = 0;
				fd.AddExtension = true;
				if (sft == "DPX")
				{
					fd.Filter = L"DPX (*.dpx)|*.dpx";
				}
				else if (sft == "SE")
				{
					fd.Filter = L"SE (*.se)|*.se";
				}
				else // if (sft == "WD")
				{
					fd.Filter = L"WD (*.wd)|*.wd";
				}
				if (fd.ShowDialog() == Windows::Forms::DialogResult::OK)
				{
					if (binaryFileRead(system2std(fd.FileName), file))
					{
						if (sft == "DPX")
						{
							if (*assetType == ".mag")
							{
								*assetData = setMagDpx(*assetData, file);
							}
							else if (*assetType == ".mdls")
							{
								*assetData = setMdlsDpx(*assetData, file);
							}
							else if (*assetType == ".moa")
							{
								*assetData = setMoaDpx(*assetData, file);
							}
							else if (*assetType == ".wpn")
							{
								*assetData = setWpnDpx(*assetData, file);
							}
							else
							{
								MessageBox::Show(this, L"Unable to import DPX to this asset type", L"HyperCrown - Import Sub-File",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}

						}
						else if (sft == "SE")
						{
							if (*assetType == ".mdls")
							{
								*assetData = setMdlsSe(*assetData, file);
							}
							else
							{
								MessageBox::Show(this, L"Unable to import SE to this asset type", L"HyperCrown - Import Sub-File",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}
						}
						else // if (sft == "WD")
						{
							if (*assetType == ".se")
							{
								std::vector<std::string> tmp;
								tmp.reserve(1);
								tmp.push_back(file);
								*assetData = setSeWds(*assetData, tmp);
							}
							else
							{
								MessageBox::Show(this, L"Unable to import WD to this asset type", L"HyperCrown - Import Sub-File",
									MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
								success = false;
							}
						}
					}
					else
					{
						MessageBox::Show(this, L"Failed to read file from file system", L"HyperCrown - Import Sub-File",
							MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
						success = false;
					}
					(*assetSubfileStatus)[i] = file != "";
				}
				else
				{
					success = false;
				}
			}
			else
			{
				MessageBox::Show(this, L"Unable to Import to this subfile type", L"HyperCrown - Import Sub-File",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
				success = false;
			}
		}
		else
		{
			MessageBox::Show(this, L"Invalid Subfile index", L"HyperCrown - Import Sub-File",
				MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			success = false;
		}
		if (success)
		{
			MessageBox::Show(this, L"Import Successful", L"HyperCrown - Export Sub-File",
				MessageBoxButtons::OK, MessageBoxIcon::Information, MessageBoxDefaultButton::Button1);
		}
		return;
	}

	private: void exportMsetSubFile(unsigned int i, std::string path)
	{
		MessageBox::Show(this, L"This feature is not currently supported", L"HyperCrown - Export Sub-File",
			MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		if (path != "" && path.back() != '\\')
		{
			path += "\\";
		}
		return;
	}

	private: void importMsetSubFile(unsigned int i)
	{
		MessageBox::Show(this, L"This feature is not currently supported", L"HyperCrown - Import Sub-File",
			MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
		return;
	}
};
}
