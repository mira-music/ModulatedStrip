#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"

//==============================================================================
// PRESET BROWSER
//==============================================================================
class PresetBrowser : public juce::Component
{
public:
    PresetBrowser(PresetManager& pm)
        : presetManager(pm)
    {
        setSize(420, 520);

        // Category list
        categoryList.setColour(
            juce::ListBox::backgroundColourId,
            juce::Colour(0xFF0F0F0F));
        categoryList.setColour(
            juce::ListBox::outlineColourId,
            juce::Colour(0xFF2A2A2A));
        categoryList.setRowHeight(28);
        categoryList.setModel(&categoryModel);
        addAndMakeVisible(categoryList);

        // Preset list
        presetList.setColour(
            juce::ListBox::backgroundColourId,
            juce::Colour(0xFF0A0A0A));
        presetList.setColour(
            juce::ListBox::outlineColourId,
            juce::Colour(0xFF2A2A2A));
        presetList.setRowHeight(36);
        presetList.setModel(&presetModel);
        addAndMakeVisible(presetList);

        // Description label
        descLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFF888888));
        descLabel.setFont(juce::Font(
            juce::FontOptions(8.5f)));
        descLabel.setJustificationType(
            juce::Justification::topLeft);
        addAndMakeVisible(descLabel);

        // Save button
        saveBtn.setButtonText("SAVE PRESET");
        saveBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF1A1200));
        saveBtn.setColour(
            juce::TextButton::textColourOnId,
            juce::Colour(0xFFE8A838));
        saveBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        saveBtn.onClick = [this] { showSaveDialog(); };
        addAndMakeVisible(saveBtn);

        // Delete button
        deleteBtn.setButtonText("DELETE");
        deleteBtn.setEnabled(false);
        deleteBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF1A0808));
        deleteBtn.setColour(
            juce::TextButton::textColourOnId,
            juce::Colour(0xFFC83020));
        deleteBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFC83020));
        deleteBtn.onClick = [this] {
            deleteSelectedPreset();
        };
        addAndMakeVisible(deleteBtn);

        // Close button
        closeBtn.setButtonText("X");
        closeBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF1A1A1A));
        closeBtn.setColour(
            juce::TextButton::textColourOnId,
            juce::Colour(0xFF888888));
        closeBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFF888888));
        closeBtn.onClick = [this] {
            setVisible(false);
        };
        addAndMakeVisible(closeBtn);

        // Nav buttons
        prevBtn.setButtonText("<");
        nextBtn.setButtonText(">");
        prevBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF111111));
        prevBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        nextBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF111111));
        nextBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        prevBtn.onClick = [this] {
            presetManager.loadPreviousPreset();
            currentName.setText(
                presetManager.getCurrentPresetName(),
                juce::dontSendNotification);
            if (onPresetLoaded) onPresetLoaded();
        };
        nextBtn.onClick = [this] {
            presetManager.loadNextPreset();
            currentName.setText(
                presetManager.getCurrentPresetName(),
                juce::dontSendNotification);
            if (onPresetLoaded) onPresetLoaded();
        };
        addAndMakeVisible(prevBtn);
        addAndMakeVisible(nextBtn);

        // Current preset name
        currentName.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFFE8A838));
        currentName.setFont(juce::Font(
            juce::FontOptions(10.0f).withStyle("Bold")));
        currentName.setJustificationType(
            juce::Justification::centred);
        addAndMakeVisible(currentName);

        // Wire models to browser
        categoryModel.browser = this;
        presetModel  .browser = this;

        refresh();
    }

    void refresh()
    {
        categories = presetManager.getCategories();
        categoryModel.categories = categories;
        categoryList.updateContent();
        categoryList.repaint();

        if (categories.size() > 0)
        {
            categoryList.selectRow(0);
            onCategorySelected(0);
        }

        currentName.setText(
            presetManager.getCurrentPresetName(),
            juce::dontSendNotification);
    }

    void onCategorySelected(int row)
    {
        if (row < 0
         || row >= static_cast<int>(categories.size()))
            return;

        selectedCategory = categories[row];
        currentPresets = presetManager
            .getPresetsForCategory(selectedCategory);
        presetModel.presets = currentPresets;
        presetList.updateContent();
        presetList.repaint();
        presetList.deselectAllRows();
        descLabel.setText("",
            juce::dontSendNotification);
        deleteBtn.setEnabled(false);
    }

    void onPresetSelected(int row)
    {
        if (row < 0
         || row >= currentPresets.size())
            return;

        auto* preset = currentPresets[row];
        descLabel.setText(preset->description,
            juce::dontSendNotification);
        deleteBtn.setEnabled(
            preset->author == "User");
    }

    void onPresetDoubleClicked(int row)
    {
        if (row < 0
         || row >= currentPresets.size())
            return;

        presetManager.loadPreset(currentPresets[row]);
        currentName.setText(
            presetManager.getCurrentPresetName(),
            juce::dontSendNotification);

        if (onPresetLoaded) onPresetLoaded();
    }

    std::function<void()> onPresetLoaded;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xFF0F0F0F));
        g.fillRoundedRectangle(b, 6.0f);

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(b, 6.0f, 1.0f);

        // Header bar
        g.setColour(juce::Colour(0xFF111111));
        g.fillRoundedRectangle(
            juce::Rectangle<float>(
                b.getX(), b.getY(),
                b.getWidth(), 40), 6.0f);
        g.fillRect(b.getX(), b.getY() + 20,
            b.getWidth(), 20.0f);

        // Amber pinstripe
        g.setColour(juce::Colour(0xFF7A4A10));
        g.fillRect(b.getX(), b.getY() + 39,
            b.getWidth(), 1.0f);

        // Title
        g.setColour(juce::Colour(0xFFE8C878));
        g.setFont(juce::Font(
            juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText("PRESET BROWSER",
            0, 0, getWidth(), 40,
            juce::Justification::centred);

        // Column labels
        g.setColour(juce::Colour(0xFF555555));
        g.setFont(juce::Font(
            juce::FontOptions(8.0f).withStyle("Bold")));
        g.drawText("CATEGORY",
            8, 44, 120, 14,
            juce::Justification::left);
        g.drawText("PRESETS",
            140, 44, 240, 14,
            juce::Justification::left);

        // Description area
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(
            juce::Rectangle<float>(
                8, getHeight() - 105.0f,
                getWidth() - 16, 60), 3.0f);
        g.setColour(juce::Colour(0xFF252525));
        g.drawRoundedRectangle(
            juce::Rectangle<float>(
                8, getHeight() - 105.0f,
                getWidth() - 16, 60), 3.0f, 1.0f);

        g.setColour(juce::Colour(0xFF444444));
        g.setFont(juce::Font(
            juce::FontOptions(7.5f).withStyle("Bold")));
        g.drawText("DESCRIPTION",
            12, getHeight() - 108, 120, 12,
            juce::Justification::left);
    }

    void resized() override
    {
        int W = getWidth();
        int H = getHeight();

        closeBtn.setBounds(W - 30, 8, 22, 22);
        prevBtn    .setBounds(8, 8, 28, 22);
        currentName.setBounds(40, 8, W - 100, 22);
        nextBtn    .setBounds(W - 60, 8, 28, 22);

        categoryList.setBounds(8, 60, 125, H - 170);
        presetList  .setBounds(140, 60, W - 148, H - 170);

        descLabel.setBounds(
            14, H - 102, W - 24, 54);

        saveBtn  .setBounds(8,   H - 36, 200, 28);
        deleteBtn.setBounds(216, H - 36, 100, 28);
    }

private:
    PresetManager& presetManager;

    juce::ListBox categoryList;
    juce::ListBox presetList;
    juce::Label   descLabel;
    juce::Label   currentName;

    juce::TextButton saveBtn;
    juce::TextButton deleteBtn;
    juce::TextButton closeBtn;
    juce::TextButton prevBtn;
    juce::TextButton nextBtn;

    juce::StringArray categories;
    juce::Array<const ModulatedStripPreset*>
        currentPresets;
    juce::String selectedCategory = "All";

    //──────────────────────────────────────────────
    // CATEGORY LIST MODEL
    //──────────────────────────────────────────────
    struct CategoryModel : public juce::ListBoxModel
    {
        PresetBrowser*    browser = nullptr;
        juce::StringArray categories;

        int getNumRows() override
        {
            return categories.size();
        }

        void paintListBoxItem(
            int row, juce::Graphics& g,
            int w, int h,
            bool selected) override
        {
            if (row < 0
             || row >= categories.size())
                return;

            if (selected)
            {
                g.setColour(
                    juce::Colour(0xFF1A1200));
                g.fillRect(0, 0, w, h);
                g.setColour(
                    juce::Colour(0xFF2A2000));
                g.drawRect(0, 0, w, h, 1);
            }

            g.setColour(selected
                ? juce::Colour(0xFFE8A838)
                : juce::Colour(0xFFAA9868));
            g.setFont(juce::Font(
                juce::FontOptions(8.5f)
                .withStyle("Bold")));
            g.drawText(categories[row],
                8, 0, w - 8, h,
                juce::Justification::centredLeft);

            g.setColour(juce::Colour(0xFF1A1A1A));
            g.drawHorizontalLine(h - 1, 0, (float)w);
        }

        void selectedRowsChanged(int row) override
        {
            if (browser)
                browser->onCategorySelected(row);
        }
    } categoryModel;

    //──────────────────────────────────────────────
    // PRESET LIST MODEL
    //──────────────────────────────────────────────
    struct PresetModel : public juce::ListBoxModel
    {
        PresetBrowser* browser = nullptr;
        juce::Array<const ModulatedStripPreset*>
            presets;

        int getNumRows() override
        {
            return presets.size();
        }

        void paintListBoxItem(
            int row, juce::Graphics& g,
            int w, int h,
            bool selected) override
        {
            if (row < 0
             || row >= presets.size())
                return;

            auto* p = presets[row];

            if (selected)
            {
                g.setColour(
                    juce::Colour(0xFF1A1200));
                g.fillRect(0, 0, w, h);
                g.setColour(
                    juce::Colour(0xFF2A2000));
                g.drawRect(0, 0, w, h, 1);
            }

            // Preset name
            g.setColour(selected
                ? juce::Colour(0xFFE8A838)
                : juce::Colour(0xFFD8D0C0));
            g.setFont(juce::Font(
                juce::FontOptions(9.0f)
                .withStyle("Bold")));
            g.drawText(p->name,
                8, 4, w - 80, 16,
                juce::Justification::centredLeft);

            // Author tag
            g.setColour(p->author == "User"
                ? juce::Colour(0xFF6688AA)
                : juce::Colour(0xFF445544));
            g.setFont(juce::Font(
                juce::FontOptions(7.0f)));
            g.drawText(p->author,
                w - 75, 4, 70, 12,
                juce::Justification::centredRight);

            // Category
            g.setColour(juce::Colour(0xFF555555));
            g.setFont(juce::Font(
                juce::FontOptions(7.5f)));
            g.drawText(p->category,
                8, 20, w - 16, 12,
                juce::Justification::centredLeft);

            // Bottom hairline
            g.setColour(juce::Colour(0xFF1A1A1A));
            g.drawHorizontalLine(h - 1, 0, (float)w);
        }

        void selectedRowsChanged(int row) override
        {
            if (browser)
                browser->onPresetSelected(row);
        }

        void listBoxItemDoubleClicked(
            int row,
            const juce::MouseEvent&) override
        {
            if (browser)
                browser->onPresetDoubleClicked(row);
        }
    } presetModel;

    //──────────────────────────────────────────────
    // SAVE DIALOG
    //──────────────────────────────────────────────
    void showSaveDialog()
    {
        auto* dialog = new juce::AlertWindow(
            "Save Preset",
            "Enter preset name:",
            juce::MessageBoxIconType::NoIcon);

        dialog->addTextEditor("name",
            presetManager.getCurrentPresetName(),
            "Preset Name:");
        dialog->addTextEditor("desc",
            "", "Description:");
        dialog->addComboBox("category",
            { "Deep House",
              "Progressive House",
              "Melodic House",
              "Drums",
              "Vocals",
              "Live Performance",
              "Mastering",
              "User" },
            "Category:");

        dialog->addButton("Save",   1,
            juce::KeyPress(
                juce::KeyPress::returnKey));
        dialog->addButton("Cancel", 0,
            juce::KeyPress(
                juce::KeyPress::escapeKey));

        dialog->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this, dialog](int result)
                {
                    if (result == 1)
                    {
                        juce::String name =
                            dialog
                            ->getTextEditorContents(
                                "name");
                        juce::String desc =
                            dialog
                            ->getTextEditorContents(
                                "desc");
                        auto* combo =
                            dialog
                            ->getComboBoxComponent(
                                "category");
                        juce::String cat =
                            combo
                            ? combo->getText()
                            : "User";

                        if (name.isNotEmpty())
                        {
                            presetManager
                                .saveUserPreset(
                                    name, cat, desc);
                            refresh();
                        }
                    }
                    delete dialog;
                }));
    }

    //──────────────────────────────────────────────
    // DELETE PRESET
    //──────────────────────────────────────────────
    void deleteSelectedPreset()
    {
        int row = presetList.getSelectedRow();
        if (row < 0 || row >= currentPresets.size())
            return;

        auto* p = currentPresets[row];
        if (p->author != "User") return;

        juce::AlertWindow::showOkCancelBox(
            juce::MessageBoxIconType::WarningIcon,
            "Delete Preset",
            "Delete \"" + p->name + "\"?",
            "Delete", "Cancel",
            nullptr,
            juce::ModalCallbackFunction::create(
                [this, name = p->name](int result)
                {
                    if (result == 1)
                    {
                        presetManager
                            .deleteUserPreset(name);
                        refresh();
                    }
                }));
    }
};

//==============================================================================
// PRESET BAR
// Compact navigation shown in plugin header
//==============================================================================
class PresetBar : public juce::Component
{
public:
    PresetBar(PresetManager& pm)
        : presetManager(pm)
    {
        prevBtn.setButtonText("<");
        prevBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF111111));
        prevBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        prevBtn.onClick = [this] {
            presetManager.loadPreviousPreset();
            update();
            if (onPresetChanged)
                onPresetChanged();
        };
        addAndMakeVisible(prevBtn);

        nameBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF0F0F0F));
        nameBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        nameBtn.onClick = [this] {
            if (onOpenBrowser)
                onOpenBrowser();
        };
        addAndMakeVisible(nameBtn);

        nextBtn.setButtonText(">");
        nextBtn.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xFF111111));
        nextBtn.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xFFE8A838));
        nextBtn.onClick = [this] {
            presetManager.loadNextPreset();
            update();
            if (onPresetChanged)
                onPresetChanged();
        };
        addAndMakeVisible(nextBtn);

        catLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFF666666));
        catLabel.setFont(juce::Font(
            juce::FontOptions(7.5f)));
        catLabel.setJustificationType(
            juce::Justification::centred);
        addAndMakeVisible(catLabel);

        update();
    }

    void update()
    {
        nameBtn.setButtonText(
            presetManager.getCurrentPresetName());
        catLabel.setText(
            presetManager.getCurrentPresetCategory(),
            juce::dontSendNotification);
    }

    void resized() override
    {
        int H = getHeight();
        int W = getWidth();
        prevBtn.setBounds(0, 2, 24, H - 4);
        nextBtn.setBounds(W - 24, 2, 24, H - 4);
        nameBtn.setBounds(26, 2,
            W - 52, (H - 4) / 2 + 2);
        catLabel.setBounds(26, H / 2,
            W - 52, H / 2 - 2);
    }

    std::function<void()> onPresetChanged;
    std::function<void()> onOpenBrowser;

private:
    PresetManager&   presetManager;
    juce::TextButton prevBtn;
    juce::TextButton nameBtn;
    juce::TextButton nextBtn;
    juce::Label      catLabel;
};