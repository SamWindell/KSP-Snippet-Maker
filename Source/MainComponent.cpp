

#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent() :
	kspFileToRead("KSP file to read", File(""), true, false, false, "*.ksp", "", "Select a KSP file to read."),
	snippetFolder("Folder to output snippets", File(""), true, true, true, "", "", "Select a folder to output snippets.")
{
	addAndMakeVisible(&titleLabel);
	titleLabel.setMultiLine(true);
	titleLabel.setText("Read a KSP file and build a snippet for each const variable, function and macro.");
	titleLabel.setReadOnly(true);
	titleLabel.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
	titleLabel.setColour(TextEditor::textColourId, Colours::white);

	addAndMakeVisible(&buildSnippets);
	buildSnippets.setButtonText("Build Snippets");
	buildSnippets.addListener(this);

	addAndMakeVisible(&kspFileToRead);
	addAndMakeVisible(&kspFileLabel);
	kspFileLabel.setText("Select a KSP file to read from:", dontSendNotification);

	addAndMakeVisible(&snippetFolder);
	addAndMakeVisible(&snippetFolderLabel);
	snippetFolderLabel.setText("Select a folder to export snippets to:", dontSendNotification);


	addAndMakeVisible(&logBox);
	logBox.setMultiLine(true);
	logBox.setText("MESSAGE LOGGER");
	logBox.setReadOnly(true);
	logBox.setScrollbarsShown(true);

	addAndMakeVisible(&buildButton);
	buildButton.setButtonText("Build Macros");
	buildButton.addListener(this);
	buildButton.setVisible(false);

    setSize (500, 420);
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colours::dimgrey);

}

void MainContentComponent::resized()
{
	titleLabel.setBounds(30, 30, getWidth() - 60, 40);

	kspFileLabel.setBounds(30, titleLabel.getBottom() + 8, getWidth() - 60, 24);
	kspFileToRead.setBounds(30, kspFileLabel.getBottom(), getWidth() - 60, 24);

	snippetFolderLabel.setBounds(30, kspFileToRead.getBottom() + 8, getWidth() - 60, 24);
	snippetFolder.setBounds(30, snippetFolderLabel.getBottom(), getWidth() - 60, 24);

	buildSnippets.setBounds(30, snippetFolder.getBottom() + 8, getWidth() - 60, 24);


	buildButton.setBounds(30, buildSnippets.getBottom() + 16, getWidth() - 60, 24);
	logBox.setBounds(30, getHeight() - 30 - 150, getWidth() - 60, 150);

}

void MainContentComponent::logMessage(const String& message)
{
	logBox.setText(logBox.getText() + "\n" + message);
}



void MainContentComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &buildSnippets)
	{
		StringArray constantVariableNameList;
		StringArray functionOrMacroList;
		StringArray functionOrMacroNames;

		logBox.setText("BUILDING SNIPPETS");
		if (kspFileToRead.getCurrentFile().existsAsFile())
		{
			File kspFile(kspFileToRead.getCurrentFile());
			StringArray kspFileText;
			
			logMessage("Reading " + kspFile.getFileName() + " ...");
			kspFile.readLines(kspFileText);

			const String internalFlag("{ ==Internal== }");

			// Find all const, macro and function
			for (int i = 0; i < kspFileText.size(); ++i)
			{
				if (kspFileText[i].contains("declare const") == true && kspFileText[i].contains("#") == false)
				{
					String t(kspFileText[i].trim());
					t = t.replace("declare const", "");
					t = t.dropLastCharacters(t.length() - t.indexOf(":="));
					t = t.replace(" ", "");
					constantVariableNameList.add(t);
				}
				else if ((kspFileText[i].trim().startsWith("macro ") == true || kspFileText[i].trim().startsWith("function ") == true) && kspFileText[i-1].trim() != internalFlag)
				{
					String t(kspFileText[i].trim());
					t = t.replace("function ", "");
					t = t.replace("macro ", "");
					t = t.replace("#", "");
					if (t.contains("->"))
						t = t.dropLastCharacters(t.length() - t.indexOf("->"));
					t = t.trim();
					functionOrMacroList.add(t);
				}
			}

			// Get the names of the functions and macros
			logMessage("Building function and macro names ...");
			for (int i = 0; i < functionOrMacroList.size(); ++i)
			{
				if (functionOrMacroList[i].contains("("))
				{
					functionOrMacroNames.add(functionOrMacroList[i].dropLastCharacters(functionOrMacroList[i].length() - functionOrMacroList[i].indexOf("(")));
				}
				else
				{
					functionOrMacroNames.add(functionOrMacroList[i]);
				}
			}

			// Make the parameters ready for the snippet, for example, turn 'name' into ${1:name}
			logMessage("Building function and macro parameters ...");
			for (int i = 0; i < functionOrMacroList.size(); ++i)
			{
				if (functionOrMacroList[i].contains("(") && functionOrMacroList[i].contains(")"))
				{
					int numberOfParameters = 1;
					for (int ii = functionOrMacroList[i].indexOf("("); ii < functionOrMacroList[i].indexOf(")"); ++ii)
					{
						if (functionOrMacroList[i].substring(ii, ii + 1) == ",")
							numberOfParameters += 1;
					}

					//DBG(String(numberOfParameters));
					StringArray currentParams;
					StringArray newParams;

					int paramStart = functionOrMacroList[i].indexOf("(");
					for (int ii = 0; ii < numberOfParameters; ++ii)
					{
						String divider(",");
						if (ii == numberOfParameters - 1)
							divider = ")";

						currentParams.add(functionOrMacroList[i].substring(paramStart + 1, functionOrMacroList[i].indexOf(paramStart + 1, divider)).trim());
						paramStart = functionOrMacroList[i].indexOf(paramStart + 1, divider);

						newParams.add("${" + String(ii + 1) + ":" + currentParams[ii] + "}");
						
					}

					for (int ii = 0; ii < currentParams.size(); ++ii)
					{
						String divider(",");
						if (ii == currentParams.size() - 1)
							divider = ")";
						functionOrMacroList.set(i, functionOrMacroList[i].replace(currentParams[ii] + divider, newParams[ii] + divider));
					}

				}
			}


			// Snippet Folder
			if (snippetFolder.getCurrentFile().isDirectory())
			{

				String snip(
					"<snippet>\n"
					"	<content><![CDATA[\n"
					"#CONTENT#\n"
					"]]></content>\n"
					"	<tabTrigger>#TITLE#</tabTrigger>\n"
					"	<scope>source.ksp</scope>\n"
					"</snippet>\n"
					);

				logMessage("Writing " + String(constantVariableNameList.size()) + " constant variable snippet files ...");
				for (int i = 0; i < constantVariableNameList.size(); ++i)
				{

					String snippetText(snip.replace("#CONTENT#", constantVariableNameList[i]).replace("#TITLE#", constantVariableNameList[i]));

					File f(snippetFolder.getCurrentFile().getFullPathName() + "/" + constantVariableNameList[i] + ".sublime-snippet");
					f.replaceWithText(snippetText);
				}

				logMessage("Writing " + String(functionOrMacroList.size()) + " functions and macros snippet files ...");
				for (int i = 0; i < functionOrMacroList.size(); ++i)
				{

					String snippetText(snip.replace("#CONTENT#", functionOrMacroList[i]).replace("#TITLE#", functionOrMacroNames[i]));

					File f(snippetFolder.getCurrentFile().getFullPathName() + "/" + functionOrMacroNames[i] + ".sublime-snippet");
					f.replaceWithText(snippetText);
				}

				logMessage("Completed Successfully.");
			}
			else
			{
				logMessage("Error finding snippet folder.");
			}

		}
		else
		{
			logMessage("Error loading KSP file.");
		}

	}

}

void MainContentComponent::addTextToFile(const StringArray& textArray, File& f)
{

}


