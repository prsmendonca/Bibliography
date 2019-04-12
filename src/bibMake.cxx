#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <latex_file1> <latex_file2> <input_bib_file> <output_bib_file>" << std::endl;
        std::cerr << "Searches for all the bibliographic citations keys" << std::endl;
        std::cerr << "in the files latex_file1, latex_file2, ...," << std::endl;
        std::cerr << "finds the corresponding bibtex entries in" << std::endl;
        std::cerr << "the file input_bib_file, and copies them" << std::endl;
        std::cerr << "to the file output_bib_file." << std::endl;
        return 1;
    }
    
    //////////////////////////////////////////////////////////////////////////////////
    // This block generates a list of citation keys found in the file latex_file
    std::set<std::string> citationList;
    std::string line;
    std::string::size_type currentCitationPosition = 0;
    for (int i=1; i<argc-2; i++) {
        currentCitationPosition = 0;
        std::ifstream latexFile(argv[i]);
        // Define the types of citations. BibLaTeX has loads!
        std::vector<std::string> citationTypes = {"\\cite", "\\nocite", "\\parencite", "\\footfullcite", "\\autocite", "\\fullcite", "\\textcite"};
        // Get all lines of the .tex file.
        while (getline(latexFile,line)) { 
            // Get the lines that have a citation
            for (std::vector<std::string>::iterator itor = citationTypes.begin(); 
                 itor != citationTypes.end(); itor++)
            {
                currentCitationPosition = line.find(*itor,currentCitationPosition);
                while (currentCitationPosition != std::string::npos) {
                    currentCitationPosition = line.find("{",currentCitationPosition);      
                    if (currentCitationPosition != std::string::npos) {
                        std::string endCharacter = ",";
                        do {
                            currentCitationPosition++;
                            std::string::size_type nextPosition = line.find_first_of(",}",currentCitationPosition);
                            std::string::size_type length = nextPosition-currentCitationPosition;
                            std::cout << "Citation key: " << line.substr(currentCitationPosition,length) << std::endl;
                            citationList.insert(line.substr(currentCitationPosition,length));
                            currentCitationPosition = nextPosition;
                            endCharacter = line.substr(nextPosition,1);
                        } while (endCharacter == ",");
                    }
                    currentCitationPosition = line.find(*itor,currentCitationPosition);
                }
                currentCitationPosition = 0;
            }
        }
        latexFile.close();
    }
    //////////////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////////////
    // This block copies the "@string{...}" lines in thei input_bib_file to the output_bib_file
    std::string::size_type currentEntryPosition;
    std::ifstream inputBibFile(argv[argc-2]);
    std::ofstream ofs(argv[argc-1], std::ios_base::out | std::ios_base::trunc);
    while (getline(inputBibFile,line)) {
        std::string originalLine = line;
        std::transform(originalLine.begin(), originalLine.end(), originalLine.begin(), (int (*)(int))std::tolower);
        currentEntryPosition = originalLine.find("@string");
        if (currentEntryPosition != std::string::npos) 
            ofs << line << std::endl;    
    }
    ofs << std::endl;
    //////////////////////////////////////////////////////////////////////////////////
    
    // Possible BiBTex entries:
    const unsigned int nEntries = 18;
    std::string entries[nEntries];
    entries[0]="article";
    entries[1]="inproceedings";
    entries[2]="incollection";
    entries[3]="inbook";
    entries[4]="proceedings";
    entries[5]="book";
    entries[6]="booklet";
    entries[7]="phdthesis";
    entries[8]="mastersthesis";
    entries[9]="techreport";
    entries[10]="manual";
    entries[11]="unpublished";
    entries[12]="misc";
    entries[13]="patent";
    entries[14]="conference";
    entries[15]="online";
    entries[16]="collection";
    entries[17]="www";
    // Rewind the get pointer to the begining of the file stream.
    inputBibFile.clear();
    inputBibFile.seekg(0,std::ios::beg);
    
    while (getline(inputBibFile,line)) {
        currentEntryPosition = 0;
        // Find the next BiBTeX entry
        currentEntryPosition = line.find("@",currentEntryPosition);
        if (currentEntryPosition != std::string::npos) {
            currentEntryPosition++;
            std::string::size_type nextPosition = line.find("{",currentEntryPosition);
            std::string::size_type length = nextPosition - currentEntryPosition;
            std::string entry = line.substr(currentEntryPosition,length);
            std::transform(entry.begin(),entry.end(),entry.begin(),::tolower);
            // If found a BiBTeX entry
            bool found = false;
            for (unsigned int i=0; i<nEntries; ++i)
                if (entries[i] == entry) {
                    found = true;
                    break;
                }
            if (found) {
                // Verify whether it is one that is cited in latex_file
                nextPosition++;
                std::string::size_type commaPosition = line.find(",",nextPosition);
                length = commaPosition-nextPosition;
                std::string key = line.substr(nextPosition,length);
                found = false;
                for (std::set<std::string>::iterator i = citationList.begin(); i != citationList.end(); i++)
                    if (*i == key) { 
                        found = true;
                        break;
                    }
                if (found) {
                    // Ok, this key is in the list. 
                    int counter = 1;
                    while (counter > 0) {
                        std::string::size_type nextBracket = line.find_first_of("}{",nextPosition);
                        if (nextBracket != std::string::npos) {
                            if (line.substr(nextBracket,1) == "}") counter--;
                            else counter++;
                            nextPosition = nextBracket+1;
                        }
                        else {
                            ofs << line << std::endl;
                            getline(inputBibFile,line);	    
                            nextPosition = 0;
                        }
                    } // Can only reach this point if the "if (nextBracket != std:string::npos)" condition is satisfied
                    ofs << line << std::endl;
                    ofs << std::endl;
                }
            }
        }
    }
    inputBibFile.close();
    ofs.close();
    
    return 0;
}
