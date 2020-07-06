
#include <cstddef>
#include <cstring>
#include <iostream>

namespace Utils {
    template<size_t Width, size_t NumMeasure, size_t NumBench>
    class ResultTable {
        private:
            char table[2+2*NumMeasure][Width+1] = {'\0'};
            static const size_t BASE_SIZE = 3;
            static const size_t EXP_SIZE = 1;
            constexpr void setupTable(){
                size_t cWide = (Width-BASE_SIZE-3-EXP_SIZE)/NumBench-1;
                size_t rem = Width-((cWide+1)*NumBench+BASE_SIZE+3+EXP_SIZE);
                
                std::cout << "line" << ' ' << __LINE__ << '\n';
                std::cout << "cwide: " << cWide << '\n';
                
                
                for(int i = 0; i<2+2*NumMeasure; ++i){
                    table[i][Width] = '\0';
                }
                
                std::cout << "line" << ' ' << __LINE__ << '\n';
                
                for(int i = 1; i<2+2*NumMeasure; i+=2){
                    std::memset(table[i], '_', sizeof(char)*Width); 
                    // fix with the remainder.
                }

                std::cout << "line" << ' ' << __LINE__ << '\n';

                for(int i = 0; i<2+2*NumMeasure; i+=2){
                    for(int c = BASE_SIZE+2+EXP_SIZE; c<Width; c+=cWide){
                        table[i][c] = '|';
                    } 
                }

                std::cout << "line" << ' ' << __LINE__ << '\n';
            }
        public:
            ResultTable(size_t sizes[NumMeasure], char* descrs[NumBench], long long (*benchs[NumBench])(long long)){
                setupTable();
            }
        
            void print(std::ostream& os){
                for(int i = 0; i<2+2*NumMeasure; ++i){
                    os << table[i] << '\n';
                }
            }
    };
}