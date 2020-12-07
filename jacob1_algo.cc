#include <vector>
#include <random>
using namespace std;

vector<int> seidal (vector<vector<int>> & a, vector<int> & x, vector<int>&b)
{
    int n = (int)a.size();
    for (int j = 0; j < n; ++j)
    {
        int d = b[j];
        for(int i = 0; i < n; ++i)
        {
            if(j != i)
            {
                d-=a[j][i] * x[i];
            }
        }
        x[j] = d/a[j][j];
    }
}

int main (int argc, char* argv[])
{
    int n = 15;
    vector<int> x(n, 0);
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 2);

    vector<vector<int>> a(n);
    vector<int>b(n, 0);
    for(int i = 0; i < n; ++i){
        vector<int> temp(n);
        for(int j = 0; j < n; ++j)
        {
            temp[j] = dis(gen);


        }
        a[i] = temp;
        b[i] = dis(gen);
    }

    vector<int> ret = seidal(a, x,b );

    // vector<int> 
}