#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>
#include <exception>
#include <complex>
#include <cmath>
#include <numbers>

template<std::size_t>
class BigNum;


class ComplexVector
{
public:
	ComplexVector(std::size_t size): ptr_(new std::complex<double>[size]{}), size_(size)
	{}
	
	ComplexVector(const ComplexVector& other): ComplexVector(other.size_)
	{
		std::memcpy(this->ptr_.get(), other.ptr_.get(),
			       	sizeof(std::complex<double>) * this->size_);
	}

	std::complex<double>& get(std::size_t index) noexcept
	{
		return *(ptr_.get() + index);
	}

	const std::complex<double>& get(std::size_t index) const noexcept 
	{
		return *(ptr_.get() + index);
	}


	std::size_t size() const noexcept
	{
		return this->size_;
	}

	ComplexVector& operator *= (const ComplexVector& other)
	{
		if(this->size() != other.size())
			throw std::runtime_error("this size != other size");

		for(std::size_t i = 0; i < this->size(); i++)
		{
			*(this->ptr_.get() + i) *= (*(other.ptr_.get()+i));
		}
		return *this;
	}

	ComplexVector operator *(const ComplexVector& other)
	{
		ComplexVector v(*this);
		v *= other;
		return v;
	}

	template<std::size_t n>
	BigNum<n> DFT()
	{
		BigNum<n> v(this->size() + 1);
		v.resize(this->size());
		for(std::size_t i = 0; i < this->size(); i++)
		{
			std::complex<double> e = 
				 std::exp(-2 * std::numbers::pi_v<double> * i / this->size() * std::complex<double>(0, 1));
			std::complex<double> eNow = e;
			double jq = this->get(0).real();
			for(std::size_t j = 1; j < this->size(); j++)
			{
				jq += (eNow * this->get(j)).real();
			      	eNow*=e;
			}
			jq /= this->size();
			v.get(i) = std::round(jq);

		}
		return v;
	}


private:
	
	void resize_(std::size_t size)
	{
		bool needResize = (this->size_ < size);

		if(needResize)
		{
			std::complex<double>* ptr = ptr_.get();
			ptr_ = std::move(
					std::unique_ptr<std::complex<double>[]>(
							new std::complex<double>[size]{}
									       )
					);
			std::memcpy(ptr_.get(), ptr, this->size_);
			this->size_ = size;
		}
	}


private:
	std::unique_ptr<std::complex<double>[]> ptr_;
	std::size_t size_;

};

template<std::size_t N = 4>
class BigNum
{
	using number = std::uint32_t;
public:
	BigNum(const std::string& str): BigNum(new number[(str.size())/N + 2]{}, str.size()/N + 2)
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
		number max = std::max(otherSize,this->size());
		this->resize(max+1); 

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
		for(; i < thisSize && sign; i++)
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
		BigNum result(thisSize + otherSize + 1);
		result.resize(thisSize + otherSize);
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

	BigNum& operator /= (std::uintmax_t num)
	{
		if(num == 0)
			throw std::runtime_error("Divide by 0");
		std::uintmax_t carry = 0;
		number* arr = this->pointerToArr_();
		number O = BigNum::pow(N);
		for(int i = this->size(); i > 0; i--)
		{
			std::uintmax_t v = (*(arr+i-1)) + carry;
			*(arr + i - 1) = static_cast<number>(v/num);
			carry = (v%num) * O;
		}
		this->optimize();
		return *this;
	}

	BigNum operator /(std::uintmax_t num)
	{
		BigNum newNum(*this);
		newNum /= num;
		return newNum;
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
		number difSize = thisSize - otherSize + 1;
		BigNum down(difSize + 2);
		number* downBuf = down.pointerToArr_();
		BigNum up(difSize + 2);
		up.resize(difSize + 1);
		number* upBuf = up.pointerToArr_();
		*(upBuf + difSize) = BigNum::pow(N)/2; 
		BigNum sumUpDown(difSize + 3);
		BigNum difUpDown(difSize + 3);
		do{
			sumUpDown.clear_();
			sumUpDown += up;
			sumUpDown += down;
			sumUpDown /= 2;

			BigNum mul = sumUpDown * other;

			difUpDown.clear_();
			difUpDown += up;
			difUpDown -= down;
			difUpDown /= 2;
			

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
	
	number& get(number index)
	{
		number* arr = this->pointerToArr_();
		return *(arr+index);
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

	static ComplexVector DFT(const BigNum& num)
	{
		ComplexVector v(num.size());
		const number* arr = num.pointerToArr_();
		for(std::size_t i = 0; i < num.size(); i++)
		{
			std::complex<double> e = 
				 std::exp(2 * std::numbers::pi_v<double> * i / num.size() * std::complex<double>(0, 1));
			std::complex<double> eNow = e;
			v.get(i) = *(arr);
			for(std::size_t j = 1; j < num.size(); j++)
			{
				v.get(i) += (eNow * std::complex<double>(*(arr + j)));
			      	eNow*=e;
			}

		}	
		return v;
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
	friend class ComplexVector;	
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
	BigNum<4> y ("7567452345");	
	BigNum<4> z ("15452");	
	BigNum<4> d = y/z;
	std::cout << static_cast<std::string>(d) << std::endl;
}
