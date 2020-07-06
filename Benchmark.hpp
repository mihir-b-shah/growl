
#include <cstddef>
#include <cstring>
#include <iostream>

namespace Utils {
    template<size_t Width, size_t NumMeasure, size_t NumBench>
    class ResultTable {
        private:
            char table[2+2*NumMeasure][Width+1];
            static const size_t BASE_SIZE = 4;
            static const size_t EXP_SIZE = 1;
            static const size_t BUF_SIZE = 6;
            
            struct ColLayout {
                int offset;
                int width;
                
                ColLayout(int o, int w){
                    offset = o;
                    width = w;
                }
            };
            
            constexpr ColLayout setupTable(){
                std::memset(table, ' ', sizeof(char)*(2+2*NumMeasure)*(1+Width));
                
                size_t cWide = (Width-BASE_SIZE-BUF_SIZE-EXP_SIZE)/NumBench-1;
                size_t rem = Width-((cWide+1)*NumBench+BASE_SIZE+BUF_SIZE+EXP_SIZE);

                for(int i = 0; i<2+2*NumMeasure; ++i){
                    table[i][Width] = '\0';
                }

                for(int i = 1; i<2+2*NumMeasure; i+=2){
                    std::memset(table[i], '-', sizeof(char)*Width); 
                    // fix with the remainder.
                }
                
                for(int i = 0; i<2+2*NumMeasure; i+=2){
                    for(int c = BASE_SIZE+BUF_SIZE+EXP_SIZE; c<Width; c+=cWide){
                        table[i][c] = '|';
                    } 
                }
                
                return ColLayout(BASE_SIZE+BUF_SIZE+EXP_SIZE, cWide);
            }
            
            struct ExpForm {
                double v1; 
                int v2;
                
                ExpForm(double one, int two){v1 = one; v2 = two;}
            };
            static constexpr ExpForm getNotation(size_t v){
                // get power.
                int power = 1;
                int ctr = 0;
                while(power <= v){
                    power *= 10;
                    ++ctr;
                }
                power /= 10;
                
                return ExpForm(static_cast<double>(v)/power,ctr-1);
            }
            static inline int min(int a, int b){
                return a<b?a:b;
            }
        public:
            ResultTable(size_t sizes[NumMeasure], const char* const descrs[NumBench], long long (*benchs[NumBench])(long long)){
                ColLayout layout = setupTable();
                
                {
                    char buf[1+layout.offset];
                    for(int i = 0; i<NumMeasure; ++i){
                        ExpForm notation = getNotation(sizes[i]);

                        _Pragma("GCC diagnostic push")
                        _Pragma("GCC diagnostic ignored \"-Wstringop-overflow=\"")

                        std::snprintf(buf, 1+layout.offset, " %.2fe%d ", notation.v1, notation.v2);
                        std::strncpy(table[2+2*i], buf, min(std::strlen(buf)-1,layout.offset-1));
                        
                        _Pragma("GCC diagnostic pop")
                    }
                }

                int ctr = 0;
                for(int i = layout.offset; ctr<NumBench; i+=layout.width){
                    _Pragma("GCC diagnostic push")
                    _Pragma("GCC diagnostic ignored \"-Wstringop-overflow=\"")
                    
                    std::strncpy(table[0]+i+2, descrs[ctr], min(std::strlen(descrs[ctr]), layout.width-1));
                    
                    _Pragma("GCC diagnostic pop")
                    ++ctr;
                }
                
                char buf[1+layout.width];
                for(int m = 0; m<NumMeasure; ++m){
                    ctr = 0;
                    for(int b = layout.offset; ctr<NumBench; b+=layout.width){
                        
                        _Pragma("GCC diagnostic push")
                        _Pragma("GCC diagnostic ignored \"-Wstringop-overflow=\"")

                        std::snprintf(buf, 1+layout.width, " %llu ", benchs[ctr](sizes[m]));
                        std::strncpy(table[2+2*m]+b+1, buf, min(std::strlen(buf)-1,layout.width-1));
                        
                        _Pragma("GCC diagnostic pop")
                        ++ctr;
                    }
                } 
            }
        
            void print(std::ostream& os){
                os << '\n';
                for(int i = 0; i<2+2*NumMeasure; ++i){
                    os << table[i] << '\n';
                }
                os << '\n';
            }
    };
}