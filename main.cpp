#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>
#include <exception>
template<std::size_t N = 4>
class BigNum
{
	using number = std::uint32_t;
public:
	BigNum(const std::string& str): BigNum(new number[str.size()/N + 2]{}, str.size()/N + 2)
	{
		number* buf = this->pointerToArr_();
		std::size_t position = 0;
		for(auto i = str.rbegin(); i != str.rend(); i++, position++)
		{
			std::size_t index = position/N;
			std::size_t num = position%N;
			if(!('0' <= (*i) && *i <= '9'))
				throw std::runtime_error("Not in base10");
			
			number result = *i - '0'; 
			for(std::size_t i = 0; i < num; i++)
			{
				result*=10;
			}
			*(buf+index) += result;
		}
		std::size_t index = str.size()/N;
		if(str.size()%N != 0)
		{
			index++;
		}
		this->counter_() = index;
	}


	BigNum(std::size_t capacity = 16) : BigNum(new number[capacity], capacity)
	{
		this->counter_() = 1;
		(*this->pointerToArr_()) = 0;	
	}

	BigNum(const BigNum& other) : BigNum(new number[other.capacity_], other.capacity_)
	{
		std::memcpy(this->arr_, other.arr_, other.capacity_*sizeof(number)); //copy
	}

	BigNum(const BigNum& other, std::size_t capacity) : BigNum(new number[capacity], capacity)
	{
		if(capacity < other.size())
		{
			std::cerr << capacity << '>' << other.size() << std::endl;
			this->~BigNum();
			throw std::runtime_error("capacity is too small to copy");
		}
		std::memcpy(this->arr_, other.arr_, other.size()*sizeof(number)); //copy
	}

	BigNum& operator=(const BigNum& other)
	{
		this->~BigNum();//destroy old

		number* newArr = new number[other.capacity_]; //allocate memomy
		std::memcpy(newArr, other.arr_, other.capacity_ * sizeof(number)); //copy
		this->arr_ = newArr; //set new arr
		this->capacity_ = other.capacity_;
		return *this;
	}

	BigNum operator-(const BigNum& other) const
	{
		BigNum newNum (*this);
		newNum-=other;
		return newNum;
	}

	BigNum& operator-=(const BigNum& other)
	{
		if(*this < other)
		{
			this->debug();
			other.debug();
			std::cout.flush();
			throw std::runtime_error("This object is less than other");
		}
		const number count = other.size();
		const number thisCount = this->size(); 
	 	number* thisBuf = this->pointerToArr_();
	       	const number* otherBuf = other.pointerToArr_();	
		bool sign = false;
		for(number i = 0; i < count; i++)
		{
			number thisValue = *(thisBuf+i);
			const number otherValue = *(otherBuf+i);
			thisValue -= (sign)?(1):(0);
			sign = thisValue < otherValue;

			if(sign)
				thisValue = BigNum::pow(N) + thisValue - otherValue;
			else
				thisValue = thisValue - otherValue;

			*(thisBuf + i) = thisValue;
		}
		if(sign)
		{
			*(thisBuf+count) -= 1;
		}
		this->optimize();
		return *this;
	}

	BigNum operator+(const BigNum& other) const
	{
		BigNum newNum (*this);
		newNum+=other;
		return newNum;
	}

	BigNum& operator+=(const BigNum& other)
	{
                number otherSize = other.size();
		if(otherSize >= this->size())
			this->resize(otherSize+1); 
		number thisCapacity = this->capacity_;
		number* thisBuf = this->pointerToArr_();
		number thisSize = this->size();
		const number* otherBuf = other.pointerToArr_();

		bool sign = false;
		number i = 0;
		for(; i < otherSize; i++)
		{
			number thisValue = *(thisBuf+i);
			number otherValue = *(otherBuf+i);
			thisValue+=otherValue;
			thisValue+=(sign)?(1):(0);
			sign = thisValue >= BigNum::pow(N);
			if(sign)
			{
				thisValue -= BigNum::pow(N);
			}
			*(thisBuf+i) = thisValue;
		}
		for(; i < (thisSize - 1) && sign; i++)
		{
			number thisValue = *(thisBuf+i);
			thisValue+=1;
			sign = thisValue >= BigNum::pow(N);
			if(sign)
			{
				thisValue -= BigNum::pow(N);
			}
			*(thisBuf+i) = thisValue;
		}
		if(sign)
		{
			*(thisBuf+i) = 1;
		}
		this->counter_() = thisSize;
		this->optimize();
		return *this;
		
	}
	
	BigNum operator*(const BigNum& other) const
	{
		number thisSize = this->size();
		number otherSize = other.size();
		number* thisBuf = this->pointerToArr_();
		const number* otherBuf = other.pointerToArr_();
		BigNum result(thisSize + otherSize + 2);
		result.resize(thisSize + otherSize + 1);
		number* resultBuf = result.pointerToArr_();
	
		
		for(std::size_t i = 0; i < otherSize; i++)
		{		
			number otherValue = *(otherBuf + i);
			number c = 0;
			for(std::size_t j = 0; j < thisSize; j++)
			{
				number thisValue = *(thisBuf + j);
				number resultValue = *(resultBuf + j + i);
				std::uintmax_t mul = otherValue * thisValue +
				       	c + resultValue;
				*(resultBuf + j + i) = mul%(BigNum::pow(N));
				c = mul/(BigNum::pow(N));
			}
			*(resultBuf + thisSize + i) = c;
		}

		result.optimize();
		return result;

	}
	
	BigNum& operator *=(const BigNum& other)
	{
		BigNum mul = (*this) * other;
		(*this) = mul;
		return *this;
	}

	
	BigNum operator / (const BigNum& other) const
	{
		if(other.isZero())
		{
			throw std::runtime_error("Divide by 0");
		}
		int cmp = this->compare(other);	
		if(cmp == 1)
		{
			return BigNum();
		}
		else if (cmp == 0)
		{
			return BigNum("1");
		}

		number thisSize = this->size();
		number otherSize = other.size();
		number difSize = thisSize - otherSize;

		BigNum down(difSize + 2);
		number* downBuf = down.pointerToArr_();

		BigNum up(difSize + 2);
		up.resize(difSize);
		number* upBuf = up.pointerToArr_();
		*(upBuf + difSize - 1) = BigNum::pow(N)/2;
		

		BigNum sumUpDown(difSize + 2);
		BigNum difUpDown(difSize + 2);
		do{
			sumUpDown.clear_();
			sumUpDown += up;
			sumUpDown += down;
			sumUpDown.div2();


			BigNum mul = sumUpDown * other;


			difUpDown.clear_();
			difUpDown += up;
			difUpDown -= down;
			difUpDown.div2();
			

			int compare = this->compare(mul);
			if(compare == 1)
			{
				up -= difUpDown;			
			}
			else if(compare == -1)
			{
				down += difUpDown;
			}
			else 
				break;


		}while(!difUpDown.isZero());
		sumUpDown.optimize();
		return sumUpDown;
	}

	BigNum& operator/=(const BigNum& other)
	{
		BigNum num = *this / other;
		*this = num;
		return *this;
	}

	int compare(const BigNum& other) const noexcept
	{
		number thisSize = this->size();
		number otherSize = other.size();
		if(thisSize > otherSize)
			return -1;
		else if(thisSize < otherSize)
			return 1;

		number* thisBuf = this->pointerToArr_();
		number* otherBuf = other.pointerToArr_();
		for(number i = 0; i < thisSize; i++)
		{
			number thisValue = *(thisBuf + (thisSize - 1) - i);
			number otherValue = *(otherBuf + (otherSize - 1) -i);
			if(thisValue > otherValue)
				return -1;

			else if (thisValue < otherValue)
				return 1;
		}
		return 0;
	}

	bool operator > (const BigNum& other) const noexcept
	{
		return this->compare(other) == -1;
	}

	bool operator < (const BigNum& other) const noexcept
	{
		return this->compare(other) == 1;
	}

	bool operator !=(const BigNum& other) const noexcept
	{
		return this->compare(other) != 0;
	}

	bool operator ==(const BigNum& other) const noexcept
	{
		return this->compare(other) == 0;
	}

	bool operator >=(const BigNum& other) const noexcept
	{
		int compare = this->compare(other);
		return (compare == -1) || (compare == 0);
	}
	bool operator <=(const BigNum& other) const noexcept
	{
		int compare = this->compare(other);
		return (compare == 1) || (compare == 0); 
	}
	


	void debug() const 
	{
		std::cout << "capacity: " << this->capacity_
			<< ' '  << "size: " << this->size() << " Num:";
		for(int i = 0; i < this->counter_(); i++)
		{
			std::cout << *(this->pointerToArr_()+i) << ' ';
		}
		std::cout << '\n';
	}

	~BigNum()
	{
		std::memset(arr_, 0, this->capacity_ * sizeof(number)); //clear
		this->capacity_ = 0;
		delete[] std::launder(this->arr_);
	}

	number size() const noexcept
	{
		return *(this->arr_);
	}

	BigNum mul(const BigNum& other)
	{
		BigNum a (*this, this->size() + other.size() + 2);
		BigNum b (other, this->size() + other.size() + 2);
		BigNum S(a.size() + b.size() + 2);
		BigNum one("1");
		while(!b.isZero())
		{
			number valueB = *(b.pointerToArr_());
			if((valueB%2) == 0)
			{
				a += a;
				b.div2();
			}
			else
			{
				S+=a;
				b-=one;
			}
		}

		return S;

	}

	operator std::string()
	{
		
		number size = this->size();
		number* buf = this->pointerToArr_();

		std::string result(this->size()*N, '0');
		for(number i = 0; i < size; i++)
		{
			number num = *(buf + size - 1 - i); 
			number O = BigNum::pow(N - 1); 
			for(int q = N-1; q >= 0 ; q-- )
			{
				number c = (num/O);
				num-=c*O;
				O/=10;
				result.at(i*N + N-1 - q) = c + '0';
			}	
			
			
		}
		return result;
	}
private:
	BigNum(number* arr, std::size_t capacity): arr_(arr), capacity_(capacity)
	{}
	
	void clear_()
	{
		this->counter_() = 1;
		*(this->pointerToArr_()) = 0;
	}

	number& counter_() const noexcept
	{
		return *(this->arr_);
	}

	number* pointerToArr_() const noexcept
	{
		return this->arr_ + 1;
	}

	void reallocate(std::size_t size)
	{
		number* newArr = new number[size+1]{}; 
		number* old = this->arr_;
		std::memcpy(newArr, old, this->capacity_ * sizeof(number));
		delete[] old;
		this->arr_ = newArr;
		this->capacity_ = size+1;
	}
	
	void resize(std::size_t size)
	{
		number thisSize = this->size();
		if(size > this->capacity_ - 1)
		{
			std::cout << "size " << size << "\ncapacity - 1: "
				<< (this->capacity_ - 1) << '\n'; 
			this->reallocate(size);
			std::cout << "resize\n";
		}
		number* thisBuf = this->pointerToArr_();
		for(;thisSize < size; thisSize++)
		{
			*(thisBuf + thisSize) = 0;
		}
		this->counter_() = thisSize;
	}

	void optimize() const noexcept
	{
		number* thisBuf = this->pointerToArr_();
		for(number i = this->size()-1; i > 0 && (*(thisBuf+i) == 0); i--)
		{
			this->counter_()--;
		}
	}

	static constexpr number pow(const number n)
	{
		number result = 1;
		for(std::size_t i = 0; i < n; i++ )
		{
			result *= 10;
		}
		return result;
	}
	
	void div2()
	{
		number* buf = this->pointerToArr_();
		number size = this->size();
		number c = 0;
		for(number i = size; i > 0; i--)
		{
			number value = *(buf + i - 1);
			*(buf + i - 1) = value/2 + c * BigNum::pow(N)/2;
			c = value%2;
		}
	}
	

	bool isZero() const
	{
		return ((this->size() == 1) &&
			(*(this->pointerToArr_()) == 0 ));
	}
	friend int main();
	
private:
	number* arr_;
	std::size_t capacity_;
};

BigNum<> operator ""_BN(const char* str)
{
	return BigNum(str);
}	

	
int main()
{
	std::uint64_t numbers;
	std::cin >> numbers;
	std::mt19937_64 gen;
	std::string numA(numbers, '0');
	std::string numB(numbers, '0');
	for(std::size_t i = 0; i < numbers; i++)
	{
		int ran = gen()%10;
		numA[i] = ran+ '0';
	       	ran = gen()%10;
		numB[i] = ran + '0';	
	}
	BigNum a(numA);
	BigNum b(numB);
	auto t0 = std::chrono::steady_clock::now();
	//iBigNum d = a.mul(b);
	auto t1 = std::chrono::steady_clock::now();
	BigNum f = a * b;
	auto t2 = std::chrono::steady_clock::now();
	//std::chrono::duration<double, std::milli> mulD = (t1 - t0);
	std::chrono::duration<double, std::milli> D = (t2 - t1);
	std::cout << D.count() << std::endl;

}
