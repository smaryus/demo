#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>

using namespace std;

double sum1(const std::vector<double>& v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

double sum2(const std::vector<double>& v)
{
    double sum = 0;
    for( auto val : v )
    {
        sum += val;
    }

    return sum;
}

int main()
{
    vector<double> vec{1,2,3};
    cout << sum1(vec) << " " << sum2(vec) << endl;
}
