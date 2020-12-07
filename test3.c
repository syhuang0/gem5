int main()
{
	int i,j,k,a=1000,b=50;
        int sum=0;
	for(i=0;i<a;i++)
	{

		for(j=0;j<b;j++)
		{
	
	
			if ((i*j)%11 > 5*(j/(i+1)) )
			{
				sum--;
				int k =2;
				k-=3;
				sum--;
		
			}
			sum++;
			sum++;
			sum++;
			sum++;
			sum++;
			sum++;
		}
		sum++;
			sum++;
			sum++;

	}
	return 0;
}
