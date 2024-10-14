#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <thread>
#include <mutex>
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
    int get_speed()
    {
        return this->speed;
    }
    void starve()
    {
        this->weight -= (int) exp(1.0*log10(weight));
    }
    void eat()
    {
        this->weight += 100;
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
        this->type = 1;
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

    inline static mutex mtx;

    void init_field(int n, int m)
    {
        field = vector< vector< Unit > >(n, vector< Unit >(m));
    }

    static void process_step(vector< vector< Unit > > &field, set< pair< int,int > >::iterator bgn, set< pair< int,int > >::iterator nd, vector< pair< int,int > > &died, vector< pair< pair< int,int >, Bacteria > > &new_bacteria)
    {
        for (set< pair< int,int > >::iterator it = bgn; it != nd; it++)
        {
            Bacteria bacterium = field[(*it).first][(*it).second].get_bacterium();
            bacterium.starve();
            int rnd = rand()%4;
            pair< int,int > prr = (*it);
            switch (rnd)
            {
            case 0:
                prr.first = min(int(field.size()-1), prr.first+1*bacterium.get_speed());
                break;
            case 1:
                prr.first = max(0, prr.first-1*bacterium.get_speed());
                break;
            case 2:
                prr.second = min(int(field[0].size()-1), prr.second+1*bacterium.get_speed());
                break;
            case 3:
                prr.second = max(0, prr.second-1*bacterium.get_speed());
            default:
                break;
            }
            Field::mtx.lock();
                died.push_back(pair< int,int >((*it).first,(*it).second));
                field[(*it).first][(*it).second].set_empty();
                if (bacterium.is_alive())
                {   
                    new_bacteria.push_back(pair< pair< int,int >, Bacteria>(prr, bacterium));
                }
            Field::mtx.unlock();
        }
        return;
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

    void make_step()
    {
        vector< pair< int,int > > died;
        vector< pair< pair< int,int >, Bacteria > > new_bacteria;
        set< pair< int,int > >::iterator half1_begin = bacteria.begin();
        set< pair< int,int > >::iterator half1_end = half1_begin;
        advance(half1_end, bacteria.size()/2);
        set< pair< int,int > >::iterator half2_begin = half1_end;
        set< pair< int,int > >::iterator half2_end = bacteria.end();
        thread thr1(process_step, ref(field), half1_begin, half1_end, ref(died), ref(new_bacteria));
        thread thr2(process_step, ref(field), half2_begin, half2_end, ref(died), ref(new_bacteria));
        bacteria_count = 0;
        thr1.join();
        thr2.join();
        for (pair< int,int > pr : died)
        {
            bacteria.erase(pr);
        }
        for (pair< pair< int,int >, Bacteria> pr : new_bacteria)
        {
            if (field[pr.first.first][pr.first.second].get_unit_type() == 1)
            {
                Bacteria other_bacterium = field[pr.first.first][pr.first.second].get_bacterium();
                if (other_bacterium.get_weight() > pr.second.get_weight())
                {
                    continue;
                }
                bacteria.insert(pr.first);
                field[pr.first.first][pr.first.second].set_bacterium(pr.second);
            }
            else if (field[pr.first.first][pr.first.second].get_unit_type() == -1)
            {
                pr.second.eat();
                field[pr.first.first][pr.first.second].set_bacterium(pr.second);
                food.erase(pr.first);
                food_count -= 1;
                bacteria.insert(pr.first);
            }
            else
            {
                bacteria.insert(pr.first);
                field[pr.first.first][pr.first.second].set_bacterium(pr.second);
                bacteria_count += 1;
            }
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
    //fld.add_food(5);
    fld.add_bacteria(5);
    fld.print_field();
    char action = getchar();
    while(action)
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
        action = getchar();
    }
    return 0;
}