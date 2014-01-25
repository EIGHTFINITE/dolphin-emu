// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official Git repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "MMIO.h"
#include "MMIOHandlers.h"

#include <functional>

namespace MMIO
{

// Base classes for the two handling method hierarchies. Note that a single
// class can inherit from both.
//
// At the moment the only common element between all the handling method is
// that they should be able to accept a visitor of the appropriate type.
template <typename T>
class ReadHandlingMethod
{
public:
	virtual ~ReadHandlingMethod() {}
	virtual void AcceptReadVisitor(ReadHandlingMethodVisitor<T>& v) const = 0;
};
template <typename T>
class WriteHandlingMethod
{
public:
	virtual ~WriteHandlingMethod() {}
	virtual void AcceptWriteVisitor(WriteHandlingMethodVisitor<T>& v) const = 0;
};

// Constant: handling method holds a single integer and passes it to the
// visitor. This is a read only handling method: storing to a constant does not
// mean anything.
template <typename T>
class ConstantHandlingMethod : public ReadHandlingMethod<T>
{
public:
	explicit ConstantHandlingMethod(T value) : value_(value)
	{
	}

	virtual ~ConstantHandlingMethod() {}

	virtual void AcceptReadVisitor(ReadHandlingMethodVisitor<T>& v) const
	{
		v.VisitConstant(value_);
	}

private:
	T value_;
};
template <typename T>
ReadHandlingMethod<T>* Constant(T value)
{
	return new ConstantHandlingMethod<T>(value);
}

// Nop: extremely simple write handling method that does nothing at all, only
// respond to visitors and dispatch to the correct method. This is write only
// since reads should always at least return a value.
template <typename T>
class NopHandlingMethod : public WriteHandlingMethod<T>
{
public:
	NopHandlingMethod() {}
	virtual ~NopHandlingMethod() {}
	virtual void AcceptWriteVisitor(WriteHandlingMethodVisitor<T>& v) const
	{
		v.VisitNop();
	}
};
template <typename T>
WriteHandlingMethod<T>* Nop()
{
	return new NopHandlingMethod<T>();
}

// Direct: handling method holds a pointer to the value where to read/write the
// data from, as well as a mask that is used to restrict reading/writing only
// to a given set of bits.
template <typename T>
class DirectHandlingMethod : public ReadHandlingMethod<T>,
                             public WriteHandlingMethod<T>
{
public:
	DirectHandlingMethod(T* addr, u32 mask) : addr_(addr), mask_(mask)
	{
	}

	virtual ~DirectHandlingMethod() {}

	virtual void AcceptReadVisitor(ReadHandlingMethodVisitor<T>& v) const
	{
		v.VisitDirect(addr_, mask_);
	}

	virtual void AcceptWriteVisitor(WriteHandlingMethodVisitor<T>& v) const
	{
		v.VisitDirect(addr_, mask_);
	}

private:
	T* addr_;
	u32 mask_;
};
template <typename T>
ReadHandlingMethod<T>* DirectRead(const T* addr, u32 mask)
{
	return new DirectHandlingMethod<T>(const_cast<T*>(addr), mask);
}
template <typename T>
ReadHandlingMethod<T>* DirectRead(volatile const T* addr, u32 mask)
{
	return new DirectHandlingMethod<T>((T*)addr, mask);
}
template <typename T>
WriteHandlingMethod<T>* DirectWrite(T* addr, u32 mask)
{
	return new DirectHandlingMethod<T>(addr, mask);
}
template <typename T>
WriteHandlingMethod<T>* DirectWrite(volatile T* addr, u32 mask)
{
	return new DirectHandlingMethod<T>((T*)addr, mask);
}

// Complex: holds a lambda that is called when a read or a write is executed.
// This gives complete control to the user as to what is going to happen during
// that read or write, but reduces the optimization potential.
template <typename T>
class ComplexHandlingMethod : public ReadHandlingMethod<T>,
                              public WriteHandlingMethod<T>
{
public:
	explicit ComplexHandlingMethod(std::function<T(u32)> read_lambda)
		: read_lambda_(read_lambda), write_lambda_(InvalidWriteLambda())
	{
	}

	explicit ComplexHandlingMethod(std::function<void(u32, T)> write_lambda)
		: read_lambda_(InvalidReadLambda()), write_lambda_(write_lambda)
	{
	}

	virtual ~ComplexHandlingMethod() {}

	virtual void AcceptReadVisitor(ReadHandlingMethodVisitor<T>& v) const
	{
		v.VisitComplex(read_lambda_);
	}

	virtual void AcceptWriteVisitor(WriteHandlingMethodVisitor<T>& v) const
	{
		v.VisitComplex(write_lambda_);
	}

private:
	std::function<T(u32)> InvalidReadLambda() const
	{
		return [](u32) {
			_dbg_assert_msg_(MEMMAP, 0, "Called the read lambda on a write "
			                            "complex handler.");
			return 0;
		};
	}

	std::function<void(u32, T)> InvalidWriteLambda() const
	{
		return [](u32, T) {
			_dbg_assert_msg_(MEMMAP, 0, "Called the write lambda on a read "
			                            "complex handler.");
		};
	}

	std::function<T(u32)> read_lambda_;
	std::function<void(u32, T)> write_lambda_;
};
template <typename T>
ReadHandlingMethod<T>* ComplexRead(std::function<T(u32)> lambda)
{
	return new ComplexHandlingMethod<T>(lambda);
}
template <typename T>
WriteHandlingMethod<T>* ComplexWrite(std::function<void(u32, T)> lambda)
{
	return new ComplexHandlingMethod<T>(lambda);
}

// Invalid: specialization of the complex handling type with lambdas that
// display error messages.
template <typename T>
ReadHandlingMethod<T>* InvalidRead()
{
	return ComplexRead<T>([](u32 addr) {
		ERROR_LOG(MEMMAP, "Trying to read from an invalid MMIO (addr=%08x)",
			addr);
		return -1;
	});
}
template <typename T>
WriteHandlingMethod<T>* InvalidWrite()
{
	return ComplexWrite<T>([](u32 addr, T val) {
		ERROR_LOG(MEMMAP, "Trying to write to an invalid MMIO (addr=%08x, val=%08x)",
			addr, (u32)val);
	});
}

// Inplementation of the ReadHandler and WriteHandler class. There is a lot of
// redundant code between these two classes but trying to abstract it away
// brings more trouble than it fixes.
template <typename T>
ReadHandler<T>::ReadHandler() : m_Method(nullptr)
{
	ResetMethod(InvalidRead<T>());
}

template <typename T>
ReadHandler<T>::ReadHandler(ReadHandlingMethod<T>* method)
	: m_Method(nullptr)
{
	ResetMethod(method);
}

template <typename T>
ReadHandler<T>::~ReadHandler()
{
}

template <typename T>
void ReadHandler<T>::Visit(ReadHandlingMethodVisitor<T>& visitor) const
{
	m_Method->AcceptReadVisitor(visitor);
}

template <typename T>
void ReadHandler<T>::ResetMethod(ReadHandlingMethod<T>* method)
{
	m_Method.reset(method);

	struct FuncCreatorVisitor : public ReadHandlingMethodVisitor<T>
	{
		std::function<T(u32)> ret;

		virtual void VisitConstant(T value)
		{
			ret = [value](u32) { return value; };
		}

		virtual void VisitDirect(const T* addr, u32 mask)
		{
			ret = [addr, mask](u32) { return *addr & mask; };
		}

		virtual void VisitComplex(std::function<T(u32)> lambda)
		{
			ret = lambda;
		}
	};

	FuncCreatorVisitor v;
	Visit(v);
	m_ReadFunc = v.ret;
}

template <typename T>
WriteHandler<T>::WriteHandler() : m_Method(nullptr)
{
	ResetMethod(InvalidWrite<T>());
}

template <typename T>
WriteHandler<T>::WriteHandler(WriteHandlingMethod<T>* method)
	: m_Method(nullptr)
{
	ResetMethod(method);
}

template <typename T>
WriteHandler<T>::~WriteHandler()
{
}

template <typename T>
void WriteHandler<T>::Visit(WriteHandlingMethodVisitor<T>& visitor) const
{
	m_Method->AcceptWriteVisitor(visitor);
}

template <typename T>
void WriteHandler<T>::ResetMethod(WriteHandlingMethod<T>* method)
{
	m_Method.reset(method);

	struct FuncCreatorVisitor : public WriteHandlingMethodVisitor<T>
	{
		std::function<void(u32, T)> ret;

		virtual void VisitNop()
		{
			ret = [](u32, T) {};
		}

		virtual void VisitDirect(T* ptr, u32 mask)
		{
			ret = [ptr, mask](u32, T val) { *ptr = val & mask; };
		}

		virtual void VisitComplex(std::function<void(u32, T)> lambda)
		{
			ret = lambda;
		}
	};

	FuncCreatorVisitor v;
	Visit(v);
	m_WriteFunc = v.ret;
}

// Define all the public specializations that are exported in MMIOHandlers.h.
MMIO_PUBLIC_SPECIALIZATIONS(, u8);
MMIO_PUBLIC_SPECIALIZATIONS(, u16);
MMIO_PUBLIC_SPECIALIZATIONS(, u32);

}
