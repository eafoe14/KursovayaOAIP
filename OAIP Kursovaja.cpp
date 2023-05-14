#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <exception>
#include <iostream>
#include <clocale>

/**
 * Ошибка со статическим текстом.
 */
class MyError : public std::exception
{
    const char* errmsg;
public:
    MyError(const char* msg) : exception(), errmsg(msg) {}
    virtual ~MyError() {}
    virtual const char* what() const noexcept { return errmsg; }
};

/**
 * Функция.
 */
class Function
{
    std::string     name;

protected:

    Function(const char* text) :
        name(std::string("y = ") + std::string(text))
    {
    }

    /**
     * Собственно значение функции, переопределить в наследниках.
     */
    virtual double f(double x) const = 0;

public:

    /**
     * Значение функции в точке x.
     */
    double calcValue(double x) const
    {
        return f(x);
    }
    /**
     * Значение производной в точке x с точностью eps.
     */
    double calcDerivation(double x, double eps) const
    {
        double dx = eps / 10.0;
        return (f(x + dx) - f(x)) / dx;
    }
    /**
     * Имя функции.
     */
    const std::string& getName() const
    {
        return name;
    }
};

/**
 * Функция y = x^2
 */
class Square : public Function
{
public:
    Square() : Function("x^2") {}
protected:
    virtual double f(double x) const
    {
        return x * x;
    }
};

/**
 * Функция y = sin(x)
 */
class Sin : public Function
{
public:
    Sin() : Function("sin(x)") {}
protected:
    virtual double f(double x) const
    {
        return sin(x);
    }
};

/**
 * Набор функций
 */
class Functions
{
    Square                          square_func;
    Sin                             sin_func;
    std::vector<const Function*>    functions;
    using Iter = std::vector<const Function*>::iterator;

public:

    Functions() : square_func(), sin_func(), functions()
    {
        functions.push_back(&square_func);
        functions.push_back(&sin_func);
    }
    /**
     * Функция по индексу.
     */
    const Function& get(int index) const
    {
        int fsize = functions.size();
        if ((index < 0) || (index >= fsize))
            throw MyError("Неверный индекс функции");
        return *functions.at(index);
    }
    /**
     * Кол-во функций.
     */
    int getSize() const
    {
        return functions.size();
    }
};

/**
 * Данные для решения задачи.
 */
class Problem
{
    int         iterations; // кол-во итераций
    int         precision;  // точность (знаков).
    double      epsilon;    // точность вычислений (epsilon).
    double      left;       // левый конец отрезка, содержащего минимум
    double      right;      // правый конец отрезка, содержащего минимум
    long double      x;          // найденный минимум


public:

    static const int ITERATION_LIMIT = 10000; // предел кол-ва итераций

    Problem() :
        iterations(0),
        precision(5),
        epsilon(pow(10, -precision)),
        left(-1.0),
        right(1.0),
        x(0.0)
    {
    }

private:

    /**
     * Значение производной функции f в точке x
     */
    double dfdx(const Function& f, double x) const
    {
        return f.calcDerivation(x, precision);
    }
    /**
     * Имеет ли функция минимум  на отрезке [left;right].
     */
    bool hasMinimum(const Function& f) const
    {
        return (f.calcDerivation(left, precision) < 0)
            && (f.calcDerivation(right, precision) > 0);
    }

public:

    double getLeft() const { return left; }
    double getRight() const { return right; }
    int getPrecision() const { return precision; }
    double getEpsilon() const { return epsilon; }
    int getIterations() const { return iterations; }
    /**
     * Установка границ отрезка.
     */
    void setBounds(double a, double b)
    {
        left = a < b ? a : b;
        right = a < b ? b : a;
    }
    /**
     * Установка точности.
     */
    void setPrecision(int prec)
    {
        precision = prec;
        epsilon = pow(10, -precision);
    }
    /**
     * Строка с отрезком.
     */
    std::string getBoundsString() const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision)
            << '[' << left << ';' << right << ']';
        return oss.str();
    }
    /**
     * Строка с точностью.
     */
    std::string getPrecisionString() const
    {
        std::ostringstream oss;
        oss << precision << " знаков после запятой ("
            << std::fixed << std::setprecision(precision)
            << epsilon << ")";
        return oss.str();
    }
    /**
     * Строка с решением.
     */
    std::string getSolutionString() const
    {
        std::ostringstream oss;
        oss << "Минимум: " << x << " (найден за " << iterations << " итераций)";
        return oss.str();
    }
    /**
     * Поиск минимума.
     * Бросает исключения при всяких ошибках.
     */
    void findMinimum(const Function& fun)
    {
        if (!hasMinimum(fun))
            throw MyError("Похоже, нет минимума на заданном отрезке!");
        double a = left;
        double b = right;
        double rfi = 2 / (1 + sqrt(5));
        double x1 = b - (b - a) * rfi;
        double x2 = a + (b - a) * rfi;
        double y1 = fun.calcValue(x1);
        double y2 = fun.calcValue(x2);
        iterations = 0;
        while (iterations < ITERATION_LIMIT) {
            ++iterations;
            if (y1 >= y2) {
                a = x1;
                x1 = x2;
                y1 = y2;
                x2 = a + (b - a) * rfi;
                y2 = fun.calcValue(x2);
            }
            else {
                b = x2;
                x2 = x1;
                y2 = y1;
                x1 = b - (b - a) * rfi;
                y1 = fun.calcValue(x1);
            }
            if (fabs(b - a) < epsilon)
            {
                x = (a + b) / 2;
                return;
            }
        }
        throw MyError("Достигнут предел кол-ва итераций!");
    }
};

/**
 * Меню взаимодействия с пользователем.
 */
class Menu
{
public:
    /**
     * Коды команд.
     */
    enum {
        CMD_QUIT,
        CMD_FUNC,
        CMD_RANGE,
        CMD_PRECISION,
        CMD_SOLVE,

        CMD_COUNT
    };

public:

    /**
     * Чтение строки с консоли.
     */
    static std::string readLine()
    {
        std::string str;
        std::getline(std::cin, str);
        return str;
    }
    /**
     * Перевод строки в число (в общем случае любой тип) T.
     */
    template< class T > static T parse(const std::string& str)
    {
        std::istringstream iss(str);
        T retval;
        iss >> retval;
        if (iss.fail())
            throw MyError("Ошибка ввода");
        return retval;
    }
    /**
     * Пауза.
     */
    static void pause()
    {
        std::cout << "Нажмите <Enter>...";
        readLine();
    }
    /**
     * Ввод данных типа T.
     */
    template< typename T > static T input(const char* prompt)
    {
        std::cout << "Введите " << prompt << ": ";
        return parse<T>(readLine());
    }
    /**
     * Выбор функции.
     */
    static int readFunction(const Functions& funcs)
    {
        std::cout << "0] Назад" << std::endl;
        for (int i = 0; i < funcs.getSize(); ++i) {
            std::cout << (i + 1) << "] "
                << funcs.get(i).getName() << std::endl;
        }
        while (true) {
            std::cout << "Команда:> ";
            int index = parse<int>(readLine());
            if ((index >= 0) && (index <= funcs.getSize())) return index;
        }
    }
    /**
     * Вывод меню и получение выбранной команды.
     */
    static int readSelection(const Function& f, const Problem& prob)
    {
        std::cout << CMD_QUIT << "] Выход из программы" << std::endl;
        std::cout << CMD_FUNC << "] Выбор функции (выбрана: "
            << f.getName() << ")" << std::endl;
        std::cout << CMD_RANGE << "] Выбор отрезка (выбран: "
            << prob.getBoundsString() << ")" << std::endl;
        std::cout << CMD_PRECISION << "] Выбор точности (выбрана: "
            << prob.getPrecisionString() << ")" << std::endl;
        std::cout << CMD_SOLVE << "] Поиск минимума" << std::endl;
        while (true) {
            std::cout << "Команда:> ";
            int index = parse<int>(readLine());
            if ((index >= CMD_QUIT) && (index < CMD_COUNT)) return index;
        }
    }
};

/**
 * Программа. Обработка ввода пользователя.
 */
class App
{
    Functions   functions;  // набор функций
    Problem     problem;    // параметры задачи
    int         current;    // выбранная функция

public:

    App() : functions(), problem(), current(0) {}

private:

    void selectFunction()
    {
        int funcid = Menu::readFunction(functions);
        if (funcid > 0) {
            current = funcid - 1;
            std::cout << "Выбрана "
                << functions.get(current).getName() << std::endl;
        }
        else {
            std::cout << "Отмена" << std::endl;
        }
    }
    /**
     * Смена  отрезка.
     */
    void selectRange()
    {
        std::cout << "Пустая строка оставит прежнее значение (в скобках)"
            << std::endl;
        double a = problem.getLeft(), b = problem.getRight();
        std::cout << "Левая граница отрезка (" << a << "): ";
        std::string s = Menu::readLine();
        if (s.length() > 0) a = Menu::parse<double>(s);
        std::cout << "Правая граница отрезка (" << b << "): ";
        s = Menu::readLine();
        if (s.length() > 0) b = Menu::parse<double>(s);
        problem.setBounds(a, b);
        std::cout << "Установлен отрезок "
            << problem.getBoundsString() << std::endl;
    }
    /**
     * Смена точности.
     */
    void setPrecision()
    {
        int prec = Menu::input<int>("точность (знаков после запятой)");
        problem.setPrecision(prec);
        std::cout << "Установлена точность "
            << problem.getPrecisionString() << std::endl;
    }
    /**
     * Решение поиск минимума.
     */
    void solve()
    {
        try {
            problem.findMinimum(functions.get(current));
            std::cout << problem.getSolutionString() << std::endl;
        }
        catch (std::exception& ex) {
            std::cerr << "* " << ex.what() << std::endl;
        }
    }

public:

    /**
     * Цикл обработки главного меню.
     */
    void run()
    {
        while (true) {
            int cmd = Menu::readSelection(functions.get(current), problem);
            switch (cmd) {
            case Menu::CMD_FUNC:        selectFunction(); break;
            case Menu::CMD_RANGE:       selectRange(); break;
            case Menu::CMD_PRECISION:   setPrecision(); break;
            case Menu::CMD_SOLVE:       solve(); break;
            default: return;
            }
            Menu::pause();
        }
    }
};

/**
 * Главная функция.
 */
int main(int argc, char** argv)
{
    try {
        setlocale(LC_ALL, "RUS");
        App app;
        app.run();
        return 0;
    }
    catch (std::exception& ex) {
        std::cerr << "* " << ex.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "* Неизвестная ошибка" << std::endl;
        return 2;
    }
}
