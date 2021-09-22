#pragma once

template <typename T>
struct ListItem
{
	T data;
	ListItem<T> *next;
	ListItem<T> *prev;
};

template <typename T>
class List
{
private:
	ListItem<T> *firstItem_;
	ListItem<T> *lastItem_;
	size_t size_;

public:
	List() : firstItem_(nullptr), lastItem_(nullptr) {}

	size_t size() { return this->size_; }

	ListItem<T> *getItem(size_t index)
	{
		ListItem<T> *item;
		size_t i;

		if (index > size_ / 2)
		{
			item = this->lastItem_;
			i = this->size_;
			while (i > 0)
			{
				i--;
				if (index == i)
					break;
				item = item->prev;
				if (item == nullptr)
					return nullptr;
			}
		}
		else
		{
			item = this->firstItem_;
			i = 0;
			while (i != index)
			{
				i++;
				item = item->next;
				if (item == nullptr)
					return nullptr;
			}
		}
		return item;
	}

	void add(T item)
	{
		ListItem<T> *i = MM::alloc<ListItem<T>>(sizeof(ListItem<T>));

		i->data = item;
		i->next = nullptr;
		i->prev = this->lastItem_;

		if (this->lastItem_ == nullptr)
		{
			this->firstItem_ = i;
			this->lastItem_ = i;
		}
		else
		{
			this->lastItem_->next = i;
			this->lastItem_ = i;
		}

		this->size_++;
	}

	void remove(size_t index)
	{
		if (index == 0)
		{
			const ListItem<T> *item = this->firstItem_;
			this->firstItem_ = this->firstItem_->next;
			MM::free(reinterpret_cast<void *>(const_cast<ListItem<T> *>(item)));
			this->size_--;
		}
		else
		{
			const ListItem<T> *item = getItem(index);
			if (item != nullptr)
			{
				if (!item->prev && !item->next)
				{
					firstItem_ = nullptr;
					lastItem_ = nullptr;
				}
				else if (!item->prev)
				{
					firstItem_ = item->next;
					item->next->prev = nullptr;
				}
				else if (!item->next)
				{
					lastItem_ = item->prev;
					item->prev->next = nullptr;
				}
				else
				{
					item->prev->next = item->next;
					item->next->prev = item->prev;
				}
				MM::free(reinterpret_cast<void *>(const_cast<ListItem<T> *>(item)));
			}
		}
	}

	template<typename Callback>
	void forEach(Callback callback)
	{
		const ListItem<T> *item = this->firstItem_;
		size_t i = 0;
		while (item != nullptr)
		{
			callback(&item->data, i);
			i++;
			item = item->next;
		}
	}
};
