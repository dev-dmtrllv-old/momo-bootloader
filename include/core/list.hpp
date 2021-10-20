#pragma once

#include "core/types.hpp"
#include "core/macros.hpp"
#include "core/mm.hpp"
#include "core/string.hpp"
#include "core/vesa.hpp"

template<typename T>
struct ListNode
{
	T data;
	ListNode<T>* prev;
	ListNode<T>* next;
	bool used = false;
} PACKED;

template<size_t Size, typename T>
struct PageList
{
	ListNode<T> nodes[Size];
	size_t freeCount = Size;
	PageList<Size, T>* prev;
	PageList<Size, T>* next;
} PACKED;

template<typename T>
class List
{
private:
	constexpr static const size_t maxNodes = ((MM::pageSize - sizeof(ptr_t)) - sizeof(size_t)) / sizeof(ListNode<T>);

	size_t count_;

	ListNode<T>* first_;
	ListNode<T>* last_;

	PageList<maxNodes, T>* pageList_;
	PageList<maxNodes, T>* lastPageList_;

	PageList<maxNodes, T>* getNewPageList()
	{
		PageList<maxNodes, T>* pl = reinterpret_cast<PageList<maxNodes, T>*>(MM::getPage());
		memset(pl, 0, MM::pageSize);
		pl->freeCount = maxNodes;
		return pl;
	}

	ListNode<T>* getFreeNode()
	{
		PageList<maxNodes, T>* last = pageList_;
		PageList<maxNodes, T>* l = pageList_;
		while (l != nullptr)
		{
			if (l->freeCount == maxNodes)
			{
				l->freeCount--;
				l->nodes[0].used = true;
				return &l->nodes[0];
			}
			else if (l->freeCount > 0)
			{
				for (size_t i = 0; i < maxNodes; i++)
				{
					if (!l->nodes[i].used)
					{
						l->freeCount--;
						l->nodes[i].used = true;
						return &l->nodes[i];
					}
				}
				// if we end here it means there was no free spots
				// lets set the freeCount to zero just to be sure
				l->freeCount = 0;
			}
			last = l;
			l = l->next;
		}
		// if no free blocks are found we need to add a new page
		PageList<maxNodes, T>* nl = getNewPageList();
		last->next = nl;
		nl->prev = last;

		nl->freeCount--;
		nl->nodes[0].used = true;

		lastPageList_ = nl;

		return &nl->nodes[0];
	}

	ListNode<T>* getNodeAt(size_t index)
	{
		ListNode<T>* n = first_;
		size_t i = 0;
		while (i++ != index && n != nullptr)
			n = n->next;
		return n;
	}

public:
	List() : count_(0), first_(nullptr), last_(nullptr)
	{
		pageList_ = getNewPageList();
		lastPageList_ = pageList_;
	}

	~List()
	{
		MM::freePage(pageList_);
	}

	size_t size()
	{
		return count_;
	}

	size_t add(T data)
	{
		ListNode<T>* n = getFreeNode();

		n->data = data;

		if (last_ == nullptr)
		{
			first_ = n;
			last_ = n;
		}
		else
		{
			last_->next = n;
			n->prev = last_;
			last_ = n;
		}
		return count_++;
	}

	T* at(size_t index)
	{
		ListNode<T>* n = getNodeAt(index);
		return n == nullptr ? nullptr : &n->data;
	}

	void insert(T data, size_t index)
	{
		if (index == 0)
		{
			ListNode<T>* n = getFreeNode();
			n->data = data;
			first_->prev = n;
			n->next = first_;
			first_ = n;
			count_++;
		}
		else if (index == count_)
		{
			add(data);
		}
		else
		{
			ListNode<T>* n = getFreeNode();
			n->data = data;
			ListNode<T>* t = getNodeAt(index);
			n->next = t;
			n->prev = t->prev;
			n->prev->next = n;
			t->prev = n;
			count_++;
		}
	}

	template<typename Callback>
	void forEach(Callback cb)
	{
		ListNode<T>* n = first_;
		size_t i = 0;
		while (n != nullptr)
		{
			cb(n->data, i++);
			n = n->next;
		}
	}

	template<typename Callback>
	bool has(Callback cb)
	{
		ListNode<T>* n = first_;
		size_t i = 0;
		while (n != nullptr)
		{
			if(cb(n->data, i++))
				return true;
			n = n->next;
		}
		return false;
	}

	template<typename Callback>
	T* find(Callback cb)
	{
		ListNode<T>* n = first_;
		size_t i = 0;
		while (n != nullptr)
		{
			if(cb(n->data, i++))
				return &n->data;
			n = n->next;
		}
		return nullptr;
	}

	void remove(size_t index)
	{
		ListNode<T>* n = getNodeAt(index);
		if (index - 1 > count_)
		{
			WARN("List: index out of bounds!");
		}
		else if (n != nullptr)
		{
			uint32_t pageAddr = MM::alignDown(reinterpret_cast<uint32_t>(n));
			PageList<maxNodes, T>* l = reinterpret_cast<PageList<maxNodes, T>*>(pageAddr);
			if (n == first_)
			{
				first_ = first_->next;
				first_->prev = nullptr;
			}
			else if (n == last_)
			{
				last_ = last_->prev;
				last_->next = nullptr;
			}
			else
			{
				n->prev->next = n->next;
				n->next->prev = n->prev;
			}

			memset(n, 0, sizeof(ListNode<T>));
			l->freeCount++;
			count_--;

			if (l->freeCount == maxNodes)
			{
				if (l == pageList_)
				{
					INFO("free first page list");
					if (pageList_->next != nullptr)
					{
						pageList_ = pageList_->next;
						MM::freePage(pageList_);
					}
				}
				else
				{
					INFO("free other page list");
					l->prev->next = l->next;
					l->next->prev = l->prev;
					MM::freePage(l);
				}
			}
		}
		else
		{
			ERROR("getNodeAt() returned nullptr!");
		}
	}

	void removeAll()
	{
		PageList<maxNodes, T>* l = pageList_->next;
		while (l != nullptr)
		{
			PageList<maxNodes, T>* next = l->next;
			INFO("free page");
			MM::freePage(l);
			l = next;
		}
		memset(pageList_, 0, MM::pageSize);
		pageList_->freeCount = maxNodes;
		lastPageList_ = pageList_;
		count_ = 0;
		first_ = nullptr;
		last_ = nullptr;
	}

	void truncate()
	{

	}
};
