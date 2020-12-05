// #include <iostream>

int main(int argc, char* argv[])
{
    int size = 70;
    int outersize = 5000;
    int arr[size];
    for(int i = 0; i < size; ++i)
    {
        arr[i] = i % 9;
    }

    int sum = 0; 
    int brr[size];

    for(int i = 0; i < outersize; i++){
        for(int j = 0; j < size; j++){
            if(arr[j] < 5){
                sum=j*i; 
                brr[j] = sum; 
            }
        }
    }
}