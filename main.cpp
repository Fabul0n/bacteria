#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <thread>
#include <set>

#define EMPTY 0
#define BACTERIUM 1
#define FOOD 2

using namespace std;

class Bacteria
{
    int weight;
    int speed;
public:
    Bacteria()
    {
        this->weight = 0;
        this->speed = 1;
    }
    Bacteria(int weight)
    {
        this->weight = weight;
        this->speed = 1;
    }
    Bacteria(int weight, int speed)
    {
        this->weight = weight;
        this->speed = speed;
    }
    Bacteria(const Bacteria& other)
    {
        this->weight = other.weight;
        this->speed = other.speed;
    }
    Bacteria& operator=(Bacteria& other)
    {
        this->weight = other.weight;
        this->speed = other.speed;
        return *this;
    }
    bool is_alive()
    {
        if (this->weight)
        {
            return 1;
        }
        return 0;
    }
    int get_weight()
    {
        return this->weight;
    }
    void set_weight(int weight)
    {
        this->weight = weight;
    }
    void starve()
    {
        this->weight -= (int) exp(1.0*log10(weight));
    }
    
    
    friend ostream& operator<<(ostream& stream, Bacteria& bacterium)
    {
        stream << bacterium.get_weight();
    }
};

class Unit
{
    int type;
    Bacteria bacterium;

public:
    Unit()
    {
        set_empty();
    }

    void set_bacterium(int weight) 
    {
        Bacteria tmp(weight);
        this->bacterium = tmp;
        this->type = 1;
    }
    void set_bacterium(int weight, int speed)
    {
        Bacteria tmp(weight, speed);
        bacterium = tmp;
        this->type = 1;
    }
    void set_bacterium(Bacteria bacterium)
    {
        this->bacterium = bacterium;
    }

    void set_food()
    {
        this->type = -1;
    }

    void set_empty()
    {
        this->type = 0;
    }

    void print_unit()
    {
        if (this->type != 1)
            cout << this->type;
        else
            cout << this->bacterium;
    }

    int get_unit_type()
    {
        return this->type;
    }

    Bacteria get_bacterium()
    {
        return this->bacterium;
    }

    int get_unit()
    {
        if (this->type == 1)
        {
            return this->bacterium.get_weight();
        }
        else
        {
            return this->type;
        }
    }

    bool is_empty()
    {
        if (this->type == 0)
        {
            return 1;
        }
        return 0;
    }
    
    bool is_food()
    {
        if (this->type == -1)
        {
            return 1;
        }
        return 0;
    }
    
    bool is_bacterium()
    {
        if (this->type > 0)
        {
            return 1;
        }
        return 0;
    }
};

class Field
{
    int n, m;
    int food_count;
    set< pair< int, int > > food;
    int bacteria_count;
    set< pair< int, int> > bacteria;
    vector< vector< Unit > > field;

    void init_field(int n, int m)
    {
        field = vector< vector< Unit > >(n, vector< Unit >(m));
    }
public:
    Field()
    {
        this->n = 0, this->m = 0;
        this->food_count = 0;
        this->bacteria_count = 0;
        init_field(n, m);
    }
    Field(int n, int m)
    {
        this->n = n;
        this->m = m;
        this->food_count = 0;
        this->bacteria_count = 0;
        init_field(n, m);
    }
    ~Field(){};

    void add_food(int n)
    {
        for (int i = 0; i < n; i++)
        {
            if (this->food.size() + this->bacteria.size() == this->n*this->m)
                break;
            int x = rand()%this->n;
            int y = rand()%this->m;
            if (this->field[x][y].is_empty())
            {
                this->field[x][y].set_food();
                this->food.insert(pair<int,int>(x,y));
                this->food_count++;
            }
            else
            {
                i--;
                continue;
            }
        }
    }

    void add_bacteria(int n, int weight_bound=1000)
    {
        for (int i = 0; i < n; i++)
        {
            if (this->food.size() + this->bacteria.size() == this->n*this->m)
                break;
            int x = rand()%this->n;
            int y = rand()%this->m;
            if (this->field[x][y].is_empty())
            {
                this->field[x][y].set_bacterium(rand()%(weight_bound-1)+1);
                this->bacteria.insert(pair< int,int >(x,y));
                this->bacteria_count++;
            }
            else
            {
                i--;
                continue;
            }
        }
    }
    void add_bacteria(int n, vector< int > weights)
    {
        assert(((void)"weights size is not equal to bacteria size", n == weights.size()));
        for (int i = 0; i < n; i++)
        {
            if (this->food.size() + this->bacteria.size() == this->n*this->m)
                break;
            int x = rand()%this->n;
            int y = rand()%this->m;
            if (this->field[x][y].is_empty())
            {
                this->field[x][y].set_bacterium(weights[i]);
                this->bacteria.insert(pair< int,int >(x,y));
                this->bacteria_count++;
            }
            else
            {
                i--;
                continue;
            }
        }
    }
    void add_bacteria(int n, vector< int > weights, vector< int > speed)
    {
        assert(((void)"weights size is not equal to bacteria size", n == weights.size()));
        for (int i = 0; i < n; i++)
        {
            if (this->food.size() + this->bacteria.size() == this->n*this->m)
                break;
            int x = rand()%this->n;
            int y = rand()%this->m;
            if (this->field[x][y].is_empty())
            {
                this->field[x][y].set_bacterium(weights[i], speed[i]);
                this->bacteria.insert(pair< int,int >(x,y));
                this->bacteria_count++;
            }
            else
            {
                i--;
                continue;
            }
        }
    }

    double dist(int x1, int y1, int x2, int y2)
    {
        return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
    }

    void make_step()
    {
        vector< pair< int,int > > died;
        for (set< pair< int,int > >::iterator it = bacteria.begin(); it != bacteria.end(); it++)
        {
            Bacteria bacterium = field[(*it).first][(*it).second].get_bacterium();
            bacterium.starve();
            int rnd = rand()%4;
            switch (rnd)
            {
            case 0:
                (*it).first;
                break;
            case 1:
                break;
            case 2:
                break;
            default:
                break;
            }
            if (bacterium.is_alive())
            {   
                field[(*it).first][(*it).second].set_bacterium(bacterium);
            }
            else
            {
                field[(*it).first][(*it).second].set_empty();
                died.push_back(pair< int,int >((*it).first,(*it).second));
            }
        }
        for (pair< int,int > pr : died)
        {
            bacteria.erase(pr);
        }
    }

    void print_field(int width=4)
    {
        system("clear");
        for(int i = 0; i < this->n; i++)
        {
            for(int j = 0; j < this->m; j++)
            {
                cout << setw(width) << field[i][j].get_unit() << " ";
            }
            cout << "\n";
        }
    }
};



int main()
{
    srand(time(0));
    Field fld(10, 10);
    Unit empty;
    fld.add_food(5);
    fld.add_bacteria(5);
    fld.print_field();
    char action;
    while(action = getchar())
    {
        if (action == '1')
        {
            fld.add_food(rand()%20);
        }
        else
        {
            fld.make_step();
            fld.print_field();

        }
    }
    return 0;
}