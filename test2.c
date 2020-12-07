int main (int argc, char * argv[])

{
    int arr[15];
    int sum = 0;
    for(int i = 0; i < 10; ++i)
    {
        if(i % 2== 0 && i % 3 == 0){
            arr[i] = 1;
        }
    }
    for (int i = 0; i < 1000; ++i){
        for (int j = 0; j < 10; ++j)
        {
            if(arr[j] == 1){
                arr[j] =0;
                sum+=1;
            }
            else{
                arr[j] =1;
            }
        }
    }
}