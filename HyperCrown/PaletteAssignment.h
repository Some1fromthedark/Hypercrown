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
	/// Summary for PaletteAssignment
	/// </summary>
	public ref class PaletteAssignment : public System::Windows::Forms::Form
	{
	private: Texture *targetTexture;
	private: System::Windows::Forms::Button^  updateButton;

	private: System::Windows::Forms::Button^  importButton;

	private: Bitmap ^bitmapTexture;
	public:
		PaletteAssignment(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			targetTexture = NULL;
		}
		PaletteAssignment(Texture *t, String ^name)
		{
			InitializeComponent();
			// Store the pointer to the texture
			targetTexture = t;
			// Make sure the texture exists
			if (targetTexture != NULL)
			{
				// Convert the texture to bmi
				Bitmap ^bm;
				unsigned int largestDim = t->width >= t->height ? t->width : t->height;
				double scale = 256.0 / largestDim;
				bitmapTexture = textureToBitmap(*t);
				bm = (Bitmap ^)bitmapTexture->Clone();
				// Set the image image size
				textureImageList->ImageSize.Width = (int)(t->width * scale);
				textureImageList->ImageSize.Height = (int)(t->height * scale);
				baseCordXNumericUpDown->Maximum = t->width - 1;
				baseCordYNumericUpDown->Maximum = t->height - 1;
				selectionWidthNumericUpDown->Maximum = t->width;
				selectionWidthNumericUpDown->Value = t->width;
				selectionHeightNumericUpDown->Maximum = t->height;
				selectionHeightNumericUpDown->Value = t->height;
				paletteNumericUpDown->Maximum = t->palette.colors.size() - 1;
				if (targetTexture->paletteIndices.size() > 0)
				{
					paletteNumericUpDown->Value = targetTexture->paletteIndices[0];
				}
				else
				{
					paletteNumericUpDown->Value = 0;
				}
				// Set color selection
				SetColorSelection(bm);
				// Add the image to the image list and list view
				textureImageList->Images->Add(bm);
				textureImageListView->Items->Add(gcnew ListViewItem(name, 0));
			}
			this->Name = L"Palette Assignment - " + name;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~PaletteAssignment()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ImageList^  textureImageList;
	private: System::Windows::Forms::ListView^  textureImageListView;
	private: System::Windows::Forms::NumericUpDown^  baseCordXNumericUpDown;
	private: System::Windows::Forms::NumericUpDown^  baseCordYNumericUpDown;
	private: System::Windows::Forms::NumericUpDown^  selectionWidthNumericUpDown;
	private: System::Windows::Forms::NumericUpDown^  selectionHeightNumericUpDown;
	private: System::Windows::Forms::NumericUpDown^  paletteNumericUpDown;
	private: System::Windows::Forms::Button^  applyButton;
	private: System::ComponentModel::IContainer^  components;
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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(PaletteAssignment::typeid));
			this->textureImageList = (gcnew System::Windows::Forms::ImageList(this->components));
			this->textureImageListView = (gcnew System::Windows::Forms::ListView());
			this->baseCordXNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->baseCordYNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->selectionWidthNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->selectionHeightNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->paletteNumericUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->applyButton = (gcnew System::Windows::Forms::Button());
			this->updateButton = (gcnew System::Windows::Forms::Button());
			this->importButton = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->baseCordXNumericUpDown))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->baseCordYNumericUpDown))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->selectionWidthNumericUpDown))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->selectionHeightNumericUpDown))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->paletteNumericUpDown))->BeginInit();
			this->SuspendLayout();
			// 
			// textureImageList
			// 
			this->textureImageList->ColorDepth = System::Windows::Forms::ColorDepth::Depth32Bit;
			this->textureImageList->ImageSize = System::Drawing::Size(256, 256);
			this->textureImageList->TransparentColor = System::Drawing::Color::Transparent;
			// 
			// textureImageListView
			// 
			this->textureImageListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->textureImageListView->LargeImageList = this->textureImageList;
			this->textureImageListView->Location = System::Drawing::Point(13, 12);
			this->textureImageListView->Name = L"textureImageListView";
			this->textureImageListView->Size = System::Drawing::Size(316, 300);
			this->textureImageListView->TabIndex = 0;
			this->textureImageListView->UseCompatibleStateImageBehavior = false;
			// 
			// baseCordXNumericUpDown
			// 
			this->baseCordXNumericUpDown->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->baseCordXNumericUpDown->Location = System::Drawing::Point(335, 13);
			this->baseCordXNumericUpDown->Name = L"baseCordXNumericUpDown";
			this->baseCordXNumericUpDown->Size = System::Drawing::Size(56, 20);
			this->baseCordXNumericUpDown->TabIndex = 1;
			this->baseCordXNumericUpDown->ValueChanged += gcnew System::EventHandler(this, &PaletteAssignment::baseCoord_ValueChanged);
			// 
			// baseCordYNumericUpDown
			// 
			this->baseCordYNumericUpDown->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->baseCordYNumericUpDown->Location = System::Drawing::Point(397, 13);
			this->baseCordYNumericUpDown->Name = L"baseCordYNumericUpDown";
			this->baseCordYNumericUpDown->Size = System::Drawing::Size(56, 20);
			this->baseCordYNumericUpDown->TabIndex = 2;
			this->baseCordYNumericUpDown->ValueChanged += gcnew System::EventHandler(this, &PaletteAssignment::baseCoord_ValueChanged);
			// 
			// selectionWidthNumericUpDown
			// 
			this->selectionWidthNumericUpDown->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->selectionWidthNumericUpDown->Location = System::Drawing::Point(335, 40);
			this->selectionWidthNumericUpDown->Name = L"selectionWidthNumericUpDown";
			this->selectionWidthNumericUpDown->Size = System::Drawing::Size(56, 20);
			this->selectionWidthNumericUpDown->TabIndex = 3;
			this->selectionWidthNumericUpDown->ValueChanged += gcnew System::EventHandler(this, &PaletteAssignment::selectionDims_ValueChanged);
			// 
			// selectionHeightNumericUpDown
			// 
			this->selectionHeightNumericUpDown->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->selectionHeightNumericUpDown->Location = System::Drawing::Point(397, 40);
			this->selectionHeightNumericUpDown->Name = L"selectionHeightNumericUpDown";
			this->selectionHeightNumericUpDown->Size = System::Drawing::Size(56, 20);
			this->selectionHeightNumericUpDown->TabIndex = 4;
			this->selectionHeightNumericUpDown->ValueChanged += gcnew System::EventHandler(this, &PaletteAssignment::selectionDims_ValueChanged);
			// 
			// paletteNumericUpDown
			// 
			this->paletteNumericUpDown->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->paletteNumericUpDown->Location = System::Drawing::Point(336, 67);
			this->paletteNumericUpDown->Name = L"paletteNumericUpDown";
			this->paletteNumericUpDown->Size = System::Drawing::Size(120, 20);
			this->paletteNumericUpDown->TabIndex = 5;
			// 
			// applyButton
			// 
			this->applyButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->applyButton->Location = System::Drawing::Point(336, 289);
			this->applyButton->Name = L"applyButton";
			this->applyButton->Size = System::Drawing::Size(120, 23);
			this->applyButton->TabIndex = 6;
			this->applyButton->Text = L"Apply Changes";
			this->applyButton->UseVisualStyleBackColor = true;
			this->applyButton->Click += gcnew System::EventHandler(this, &PaletteAssignment::applyButton_Click);
			// 
			// updateButton
			// 
			this->updateButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->updateButton->Location = System::Drawing::Point(336, 94);
			this->updateButton->Name = L"updateButton";
			this->updateButton->Size = System::Drawing::Size(120, 23);
			this->updateButton->TabIndex = 7;
			this->updateButton->Text = L"Update Selection";
			this->updateButton->UseVisualStyleBackColor = true;
			this->updateButton->Click += gcnew System::EventHandler(this, &PaletteAssignment::updateButton_Click);
			// 
			// importButton
			// 
			this->importButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->importButton->Location = System::Drawing::Point(336, 124);
			this->importButton->Name = L"importButton";
			this->importButton->Size = System::Drawing::Size(120, 23);
			this->importButton->TabIndex = 8;
			this->importButton->Text = L"Import to Selection";
			this->importButton->UseVisualStyleBackColor = true;
			this->importButton->Click += gcnew System::EventHandler(this, &PaletteAssignment::importButton_Click);
			// 
			// PaletteAssignment
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(465, 324);
			this->Controls->Add(this->importButton);
			this->Controls->Add(this->updateButton);
			this->Controls->Add(this->applyButton);
			this->Controls->Add(this->paletteNumericUpDown);
			this->Controls->Add(this->selectionHeightNumericUpDown);
			this->Controls->Add(this->selectionWidthNumericUpDown);
			this->Controls->Add(this->baseCordYNumericUpDown);
			this->Controls->Add(this->baseCordXNumericUpDown);
			this->Controls->Add(this->textureImageListView);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"PaletteAssignment";
			this->Text = L"Palette Assignment";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->baseCordXNumericUpDown))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->baseCordYNumericUpDown))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->selectionWidthNumericUpDown))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->selectionHeightNumericUpDown))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->paletteNumericUpDown))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void applyButton_Click(System::Object^  sender, System::EventArgs^  e) {
		this->DialogResult = System::Windows::Forms::DialogResult::OK;
		this->Close();
		return;
	}
	private: System::Void selectionDims_ValueChanged(System::Object ^sender, System::EventArgs^ e) {
		// Convert the texture to bmi
		Bitmap ^bm = (Bitmap ^)bitmapTexture->Clone();
		// Set color selection
		SetColorSelection(bm);
		textureImageList->Images->Clear();
		textureImageList->Images->Add(bm);
		return;
	}

	private: System::Void baseCoord_ValueChanged(System::Object ^sender, System::EventArgs ^e) {
		unsigned int wv = (unsigned int)selectionWidthNumericUpDown->Value, hv = (unsigned int)selectionHeightNumericUpDown->Value, x = (unsigned int)baseCordXNumericUpDown->Value, y = (unsigned int)baseCordYNumericUpDown->Value, ind = y * targetTexture->width + x;
		// Update the maximums for the selection dims
		selectionWidthNumericUpDown->Maximum = targetTexture->width - x;
		selectionHeightNumericUpDown->Maximum = targetTexture->height - y;
		if (ind < targetTexture->paletteIndices.size())
		{
			paletteNumericUpDown->Value = targetTexture->paletteIndices[ind];
		}
		else
		{
			paletteNumericUpDown->Value = 0;
		}
		// Check if the value didn't change
		if (selectionWidthNumericUpDown->Value == wv && selectionHeightNumericUpDown->Value == hv)
		{
			Bitmap ^bm = (Bitmap ^)bitmapTexture->Clone();
			// Set color selection
			SetColorSelection(bm);
			textureImageList->Images->Clear();
			textureImageList->Images->Add(bm);
		}
		return;
	}

	private: System::Void SetColorSelection(Bitmap ^bm) {
		const int argb = 0xFFFF00FF;
		for (unsigned int i = 0; i < (unsigned int)selectionWidthNumericUpDown->Value; i++)
		{
			for (unsigned int j = 0; j < (unsigned int)selectionHeightNumericUpDown->Value; j++)
			{
				if (i == 0 || i == ((unsigned int)selectionWidthNumericUpDown->Value - 1) || j == 0 || j == ((unsigned int)selectionHeightNumericUpDown->Value - 1))
				{
					bm->SetPixel((unsigned int)baseCordXNumericUpDown->Value + i, (unsigned int)baseCordYNumericUpDown->Value + j, Color::FromArgb(argb));
				}
			}
		}
		return;
	}
	private: System::Void updateButton_Click(System::Object^  sender, System::EventArgs^  e) {
		Bitmap ^bm;
		for (unsigned int i = 0; i < (unsigned int)selectionHeightNumericUpDown->Value; i++)
		{
			for (unsigned int j = 0; j < (unsigned int)selectionWidthNumericUpDown->Value; j++)
			{
				unsigned int ind = ((unsigned int)baseCordYNumericUpDown->Value + i) * targetTexture->width + ((unsigned int)baseCordXNumericUpDown->Value + j);
				targetTexture->paletteIndices[ind] = (unsigned int)paletteNumericUpDown->Value;
			}
		}
		bitmapTexture = textureToBitmap(*targetTexture);
		bm = (Bitmap ^)bitmapTexture->Clone();
		SetColorSelection(bm);
		textureImageList->Images->Clear();
		textureImageList->Images->Add(bm);
		return;
	}
	private: System::Void importButton_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog fd;
		fd.Filter = L"PNG (*.png)|*.png";
		fd.FilterIndex = 0;
		if (fd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
		{
			Texture t;
			std::stringstream ss;
			if (readTexture(system2std(fd.FileName), t))
			{
				Bitmap ^bm;
				unsigned int baseX = (unsigned int)baseCordXNumericUpDown->Value, baseY = (unsigned int)baseCordYNumericUpDown->Value, width = (unsigned int)selectionWidthNumericUpDown->Value, height = (unsigned int)selectionHeightNumericUpDown->Value, numPalettes = targetTexture->palette.colors.size();
				t.indexWidth = targetTexture->indexWidth;
				// Check if the imported image is smaller than the selection
				if (width > t.width)
				{
					width = t.width;
					selectionWidthNumericUpDown->Value = width;
				}
				if (height > t.height)
				{
					height = t.height;
					selectionHeightNumericUpDown->Value = height;
				}
				// Clear the palettes referenced by the selection
				for (unsigned int i = 0; i < height; i++)
				{
					for (unsigned int j = 0; j < width; j++)
					{
						unsigned int targetInd = (baseY + i) * targetTexture->width + (baseX + j), paletteIndex = targetTexture->paletteIndices[targetInd];
						targetTexture->palette.colors[paletteIndex].clear();
					}
				}
				// Fill the palettes referenced by the selection and update indices in the selection
				for (unsigned int i = 0; i < height; i++)
				{
					for (unsigned int j = 0; j < width; j++)
					{
						unsigned int ind = i * t.width + j, targetInd = (baseY + i) * targetTexture->width + (baseX + j), paletteIndex = targetTexture->paletteIndices[targetInd], k;
						KHColor ci = t.palette.colors.back()[t.indices[ind]];
						for (k = 0; k < targetTexture->palette.colors[paletteIndex].size(); k++)
						{
							KHColor cp = targetTexture->palette.colors[paletteIndex][k];
							if (ci == cp)
							{
								targetTexture->indices[targetInd] = k;
								break;
							}
						}
						if (k == targetTexture->palette.colors[paletteIndex].size())
						{
							targetTexture->indices[targetInd] = k;
							targetTexture->palette.colors[paletteIndex].push_back(ci);
							k++;
						}
					}
				}
				// Pad any partially filled palettes to 256 colors
				for (unsigned int i = 0; i < numPalettes; i++)
				{
					for (unsigned int j = targetTexture->palette.colors[i].size(); j < 256; j++)
					{
						KHColor c;
						c.r = 0;
						c.g = 0;
						c.b = 0;
						c.a = 0;
						targetTexture->palette.colors[i].push_back(c);
					}
				}
				textureImageList->Images->Clear();
				// Intialize the bitmap
				bitmapTexture = textureToBitmap(*targetTexture);
				// Create the selection
				bm = (Bitmap ^)bitmapTexture->Clone();
				SetColorSelection(bm);
				textureImageList->Images->Add(bm);
			}
			else
			{
				MessageBox::Show(this, L"Failed to Read Image File", L"HyperCrown - Import Texture",
					MessageBoxButtons::OK, MessageBoxIcon::Error, MessageBoxDefaultButton::Button1);
			}
		}
		return;
	}
};
}
