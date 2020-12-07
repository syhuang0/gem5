int main (int argc, char * argv[])

{
    int size = 170;
    int arr[size];
    int sum = 0;
    int b = 10;
    int c = 15; 
    for(int i = 0; i < size; ++i)
    {
        if(i % 2== 0 && i % 3 == 0){
            arr[i] = i;
        }
    }
    for (int i = 0; i < b; ++i){
        for (int j = 0; j < c; ++j)
        {
            if(arr[j*(i+1)] > 0){
                arr[j] = -arr[j] + sum;
                sum+=1;
            }
            else{
                arr[j] =arr[j] + sum*j;
            }
        }
    }
}