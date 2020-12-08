#include <algorithm>

int main (int argc, char * argv[])

{
    int arr[15];
    int sum = 0;
    for(int i = 0; i < 15; ++i)
    {
        if(i % 2== 0 && i % 3 == 0){
            arr[i] = i;
        }
    }

    for (int i = 0; i < 1000; ++i){
        for (int j = 1; j < 11; ++j)
        {
            if(arr[j] % 6 == 0){
                // arr[j] =0;
                sum+=arr[j];
                std::swap(arr[j], arr[j-1]);
            }
            int t = 0;
            t = j*i;
            sum -=t;
            sum -=t;
            sum -=t;
            sum -=t;
            
        }
    }
}