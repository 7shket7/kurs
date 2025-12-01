#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <string>
#include <vector>

class DataProcessor {
private:
    std::vector<std::vector<double>> vectors;
    
public:
    bool readVectorsFromFile(const std::string& filename);
    bool validateData() const;
    std::vector<char> convertToBinary() const;
    bool saveResults(const std::string& filename, const std::vector<double>& results) const;
    const std::vector<std::vector<double>>& getVectors() const { return vectors; }
    size_t getVectorsCount() const { return vectors.size(); }
};

#endif // DATAPROCESSOR_H
