

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public Component, public Button::Listener

{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&);
    void resized();
	void buttonClicked(Button* buttonThatWasClicked);

private:

	TextEditor titleLabel;
	Label kspFileLabel;
	FilenameComponent kspFileToRead;
	Label snippetFolderLabel;
	FilenameComponent snippetFolder;


	TextButton buildSnippets;

	TextButton buildButton;

	TextEditor logBox;
	void logMessage(const String& message);

	void addTextToFile(const StringArray& textArray, File& f);;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
