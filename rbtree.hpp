////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Реализация классов красно-черного дерева
/// \author    Kupchenko Viktor
/// \version   0.1.0
/// \date      01.05.2017
///            This is a part of the course "Algorithms and Data Structures" 
///            provided by  the School of Software Engineering of the Faculty 
///            of Computer Science at the Higher School of Economics.
///
/// "Реализация" (шаблонов) методов, описанных в файле rbtree.h
///
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>        // std::invalid_argument


namespace xi
{


//==============================================================================
// class RBTree::Node
//==============================================================================


    template<typename Element, typename Compar>
    RBTree<Element, Compar>::Node::~Node()
    {
        if (_left)
            delete _left;
        if (_right)
            delete _right;
    }


    template<typename Element, typename Compar>
    typename RBTree<Element, Compar>::Node *RBTree<Element, Compar>::Node::setLeft(Node *lf)
    {
        // Предупреждаем повторное присвоение
        if (_left == lf)
            return nullptr;

        // Если новый левый — действительный элемент
        if (lf)
        {
            // Если у него был родитель
            if (lf->_parent)
            {
                // Ищем у родителя, кем был этот элемент, и вместо него ставим бублик
                if (lf->_parent->_left == lf)
                    lf->_parent->_left = nullptr;
                else
                    // Доп. не проверяем, что он был правым, иначе нарушение целостности
                    lf->_parent->_right = nullptr;
            }

            // Задаем нового родителя
            lf->_parent = this;
        }

        // Если у текущего уже был один левый — отменяем его родительскую связь и вернем его
        Node *prevLeft = _left;
        _left = lf;

        if (prevLeft)
            prevLeft->_parent = nullptr;

        return prevLeft;
    }


    template<typename Element, typename Compar>
    typename RBTree<Element, Compar>::Node *RBTree<Element, Compar>::Node::setRight(Node *rg)
    {
        // Предупреждаем повторное присвоение
        if (_right == rg)
            return nullptr;

        // Если новый правый — действительный элемент
        if (rg)
        {
            // Если у него был родитель
            if (rg->_parent)
            {
                // Ищем у родителя, кем был этот элемент, и вместо него ставим бублик
                if (rg->_parent->_left == rg)
                    rg->_parent->_left = nullptr;
                else // Доп. не проверяем, что он был правым, иначе нарушение целостности
                    rg->_parent->_right = nullptr;
            }

            // Задаем нового родителя
            rg->_parent = this;
        }

        // Если у текущего уже был один левый — отменяем его родительскую связь и вернем его
        Node *prevRight = _right;
        _right = rg;

        if (prevRight)
            prevRight->_parent = nullptr;

        return prevRight;
    }


//==============================================================================
// class RBTree
//==============================================================================

    template<typename Element, typename Compar>
    RBTree<Element, Compar>::RBTree()
    {
        _root = nullptr;
        _dumper = nullptr;
    }


    template<typename Element, typename Compar>
    RBTree<Element, Compar>::~RBTree()
    {
        // Удаляем корень, который удаляет всех детей и так до листьев
        if (_root)
            delete _root;
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::deleteNode(Node *nd)
    {
        // Если переданный узел не существует, просто ничего не делаем, т.к. в вызывающем проверок нет
        if (!nd)
            return;
        if (nd == _root)
            _root = nullptr;
        else
            (nd->isLeftChild() ? nd->_parent->_left : nd->_parent->_right) = nullptr;
        nd->_parent = nullptr;

        // Потомков убьет в деструкторе
        delete nd;
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::remove(const Element &key)
    {
        // Узел, который мы хотим удалить
        const Node *delNode = find(key);

        if (!delNode)
            throw new std::invalid_argument("Node with this key doesn't exist!");

        // curNode - узел для запоминания цвета и для замены удаляемого
        // rebNode - узел для утряски дерева
        Node *curNode = (Node *) delNode;
        Node *rebNode;
        // Чтобы утряска выполнялась верно и мы не получали nullptr
        // вместо nullptr будет создаваться фейковый узел, который в последствии будет удалён
        bool delRebNode = false;

        Color col = curNode->_color;

        /* Если детей нет, то будем делать утряску текущей ноды, а после её удалим
         * Как вариант можно в следующих двух кейсах создавать ноду, если она оказалась nullptr
         * Но как по мне, куда легче просто оставить текущую ноду */
        if (!delNode->_left && !delNode->_right)
        {
            rebNode = new Node();
            delRebNode = true;
            transplant(delNode, rebNode);
        }// Если нет левого ребенка, меняем с правым,
        else if (!delNode->_left)
        {
            rebNode = delNode->_right;
            transplant(delNode, delNode->_right);
        }// Иначе если нет правого ребенка, меняем с левым,
        else if (!delNode->_right)
        {
            rebNode = delNode->_left;
            transplant(delNode, delNode->_left);
        }// Иначе находим младшего потомка у правого ребенка delNode.
        else
        {
            curNode = delNode->_right;
            while (curNode->_left)
                curNode = curNode->_left;

            col = curNode->_color;
            rebNode = curNode->_right;
            // Если ребенка не было, то создаем его (опять же чтобы не встрять с nullptr)
            if (!rebNode)
            {
                rebNode = new Node();
                rebNode->_parent = curNode;
                curNode->_right = rebNode;
                delRebNode = true;
            }

            if (curNode->_parent != delNode)
            { /*  Картина примерно такая
               *      5        7
               *    2   10 -> 2 10
               *       7
               */
                transplant(curNode, curNode->_right);
                curNode->_right = delNode->_right;
                curNode->_right->_parent = curNode;
            }
            // Теперь выталкиваем эту ноду на место удаляемой
            // И отдаем ей всех левых детей.
            // reminder: у ноды, которую мы поставили, нет левых детей, потому что она самая левая среди правых.
            transplant(delNode, curNode);
            curNode->_left = delNode->_left;
            curNode->_left->_parent = curNode;
            curNode->_color = delNode->_color;
        }

        // Если не было 2 детей и был черный цвет
        // Или если у ноды, поставленной на место бывшей, черный цвет,
        // То мы могли нарушить баланс, а значит надо потрясти дерево.
        if (col == BLACK)
            rebalanceDel(rebNode);

        // Если мы достали снизу красную вершину, то утряска производиться не будет, но корень покрасить надо.
        _root->setBlack();

        // Если мы делали утряску относительно nullptr листа, то надо убить его
        if (delRebNode)
            deleteNode(rebNode);
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::transplant(const Node *before, Node *after)
    {
        if (!before)
            throw new std::invalid_argument("Can't transplant nullptr!");

        // Тернарка в тернарке тут выглядит слишком страшно, поэтому оставлю if-else
        if (!before->_parent)
            _root = after;
        else
            (before->isLeftChild() ? before->_parent->_left : before->_parent->_right) = after;

        if (after)
            after->_parent = before->_parent;
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::rebalanceDel(Node *start)
    {
        /* Стоит напомнить, что у нас либо не было 2 детей и был черный цвет
        * Тогда переданный узел - это тот самый ребенок, который пришел на место удаленной ноды
        * Либо мы достали с самого низа черную ноду, а утряску делаем относительно её бывшего правого ребенка */
        Node *cur = start;
        Node *bro;

        // Мы хотим исправить 3 проблемы ( B <-> black; R <-> red )
        /* 1. После удаления корень стал красным
         *   x(B)  del(x)    y(R)
         *  y(R)     ->
         *
         * Тут все совсем просто - цикл не выполнится т.к. мы уже в корне
         * А последняя строка покрасит корень в черный
         */
        /* 2. Если появились 2 красных подряд
         *    a(B)      del(a)       f(B)
         *b(B)    c(R)     ->    b(B)    c(R)
         *     f(B)  d(B)              g(R) d(B)
         *       g(R)
         */
        /* 3. Удаление ноды изменяет количество черных вершин до листа
         *    a(B)      del(a)       f(B)
         *b(B)    c(R)     ->    b(B)    c(R)
         *     f(B)  d(B)             null  d(B)
         *
         *  f->c->null содержит 1 черную вершину,
         *  в то время как путь до остальных листьев содержит по 2 черных вершины
         */

        /* Удалив черную ноду, мы повесили на всех её потомков дополнительный черный цвет
         * Чтобы отразить это, немножко пересмотрим параметры наших цветов в методе
         * cur->isBlack значит, что cur черный + дополнительный черный от потерянного родителя
         * cur->isRed значит, что cur красный + дополнительный черный от потерянного родителя
         *
         * Если cur->isRed, то мы можем завершать цикл, т.к. в конце метода cur красится в черный,
         * А значит баланс будет восстановлен.
         */
        while (cur != _root && cur->isBlack())
        {   /* Мои любимые тернарки
            !!! В комментариях рассматривается только левый случай, но правый полностью зеркален !!! */

            bool isLeft = cur->isLeftChild();
            // Это брат
            bro = isLeft ? cur->_parent->_right : cur->_parent->_left;

            /* Если брат есть и при том он красный,
             * То сводим эту ситуацию к остальным
             * Т.е. делаем брата черным */
            if (bro && bro->isRed())
            {
                // Красим семью и проворачиваем брата вверх
                bro->setBlack();
                cur->_parent->setRed();
                isLeft ? rotLeft(cur->_parent) : rotRight(cur->_parent);
                bro = isLeft ? cur->_parent->_right : cur->_parent->_left;
            }

            /* Если у брата два черных ребенка, то мы его самого красим в красный (nullptr это тоже черные дети)
             * Таким образом мы "забираем" один черный от себя и от брата, после чего переходим к отцу
             * Если отец был красным, то цикл заканчивается и он красится в черный
             * Иначе история повторяется */
            if ((!bro->_left || bro->_left->isBlack()) && (!bro->_right || bro->_right->isBlack()))
            {
                bro->setRed();
                cur = cur->_parent;
            } else
            {
                /* Если у брата правые дети - черные (или nullptr), и есть левый ребенок
                 * (который красный - мы это выяснили на прошлом шаге)
                 * То поворачиваем левым ребенком вверх.
                 * Это нам гарантирует следующий кейс - левый ребенок черный, а правый - красный */
                if (isLeft ? (bro->_left && (!bro->_right || bro->_right->isBlack())) :
                    (bro->_right && (!bro->_left || bro->_left->isBlack())))
                {
                    (isLeft ? bro->_left : bro->_right)->setBlack();
                    bro->setRed();
                    isLeft ? rotRight(bro) : rotLeft(bro);
                    bro = isLeft ? cur->_parent->_right : cur->_parent->_left;
                }

                /* Брат черный;
                 * Левый ребенок брата - черный/nullptr, правый - красный;
                 * Красим брата в цвет папы, а папу в черный
                 * Поворачиваем влево относительно папы, тогда брат вылезает вверх,
                 * Папа забирает на себя лишний черный цвет, заканчиваем цикл. */
                bro->_color = cur->_parent->_color;
                cur->_parent->setBlack();
                /* У брата ТОЧНО есть правый ребенок, поэтому сможем выполнить поворот
                 * (если бы не было ни одного, выполнился бы второй иф,
                 * если был бы только левый, то чуть выше мы переводим эту ситуацию к правому ребенку). */
                (isLeft ? bro->_right : bro->_left)->setBlack();
                isLeft ? rotLeft(cur->_parent) : rotRight(cur->_parent);
                cur = _root;
            }
        }

        cur->setBlack();
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::insert(const Element &key)
    {
        Node *newNode = insertNewBstEl(key);

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_INS, this, newNode);

        rebalance(newNode);

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_INSERT, this, newNode);

    }


    template<typename Element, typename Compar>
    const typename RBTree<Element, Compar>::Node *RBTree<Element, Compar>::find(const Element &key)
    {
        Node *cur = nullptr;
        Node *next = _root;
        while (next)
        {
            cur = next;

            if (cur->_key == key)  // Если в дереве есть такое значение - возвращаем узел с этим значением
                return cur;

            // Если значение меньше, то надо спускаться влево, иначе вправо
            next = _compar(key, cur->_key) ? next->_left : next->_right;
        }

        // Если цикл закончился, значит мы спустились до листьев дерева, а следовательно такого значения нет
        return nullptr;
    }


    template<typename Element, typename Compar>
    typename RBTree<Element, Compar>::Node *
    RBTree<Element, Compar>::insertNewBstEl(const Element &key)
    {
        Node *newNode = new Node(key);
        if (!_root)
        {
            _root = newNode;
            return newNode;
        }

        /* Ищем место для элемента */
        Node *cur = nullptr;
        Node *next = _root;
        while (next)
        {
            cur = next;

            if (cur->_key == newNode->_key)  // Если в дереве есть такое значение - кидаем исключение
                throw std::invalid_argument("Node with this value already exist!");

            // Если значение меньше, то надо спускаться влево, иначе вправо
            next = _compar(newNode->_key, cur->_key) ? next->_left : next->_right;
        }

        (_compar(newNode->_key, cur->_key) ? cur->_left : cur->_right) = newNode;
        newNode->_parent = cur;

        return newNode;
    }

    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::rebalance(Node *nd)
    {

        if (_root == nd)
            return;

        nd->setRed();

        if (nd->_parent->isBlack())
            return;

        // Пока папа красный
        while (nd->_parent->isRed())
        {
            // Является ли папа левым ребенком
            bool isLeft = nd->_parent->isLeftChild();

            // Если дядя красный (да-да, так выглядит дядя; если папа левый, то он правый, иначе наоборот)
            Node *unc = isLeft ? nd->_parent->_parent->_right : nd->_parent->_parent->_left;
            if (unc && unc->isRed())
            {   // Перекрашиваем семью
                nd->_parent->setBlack();
                unc->setBlack();
                nd->_parent->_parent->setRed();
                nd = nd->_parent->_parent; // Переходим к дедушке и проверяем его семью
                // Если это корень, или если мы ребенок корня, то проверка закончена
                // Корень гарантированно покрасится в черный в конце, а ребенок у корня может быть любой
                if (!nd->_parent || nd->_parent->isBlack())
                    break;
            } else
            {// Тернарки схлопывают 2 случая в один c:
                /* Если у нас не прямая ветка, а изгиб, то выворачиваем ребенка наверх, чтобы выпрямить
                 *          10         10  |     5          5
                 *        5     ->    8    |       10   ->    8
                 *          8       5      |      8             10
                 */
                if (isLeft ? nd == nd->_parent->_right : nd == nd->_parent->_left)
                {
                    nd = nd->_parent;
                    isLeft ? rotLeft(nd) : rotRight(nd);
                }
                // А после спокойно делаем поворот
                nd->_parent->setBlack();
                nd->_parent->_parent->setRed();
                isLeft ? rotRight(nd->_parent->_parent) : rotLeft(nd->_parent->_parent);
            }
        }

        _root->setBlack();
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::rotLeft(typename RBTree<Element, Compar>::Node *nd)
    {
        // Правый потомок, который станет после левого поворота "выше"
        Node *y = nd->_right;

        if (!y)
            throw std::invalid_argument("Can't rotate left since the right child is nil");

        /* Перераспределение семейных связей */
        // y станет родителем для nd
        y->_parent = nd->_parent;

        // Если это не корень дерева
        if (y->_parent != nullptr)
            // Если nd был левым ребенком, то теперь левым ребенком будет y
            if (nd->_parent->_left == nd)
                nd->_parent->_left = y;
                // Иначе правым
            else nd->_parent->_right = y;

        /* Поворот влево */
        //      nd
        //        y
        //       f g
        nd->_right = y->_left;
        if (nd->_right)
            nd->_right->_parent = nd;

        //       y
        //    nd   g
        //      f
        nd->_parent = y;
        y->_left = nd;

        if (_root->_parent)
            _root = _root->_parent;

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_LROT, this, nd);
    }


    template<typename Element, typename Compar>
    void RBTree<Element, Compar>::rotRight(typename RBTree<Element, Compar>::Node *nd)
    {
        // Левый потомок, который станет после правого поворота "выше"
        Node *y = nd->_left;

        if (!y)
            throw std::invalid_argument("Can't rotate left since the right child is nil");

        /* Перераспределение семейных связей */
        // y станет родителем для nd
        y->_parent = nd->_parent;

        // Если это не корень дерева
        if (y->_parent != nullptr)
            // Если nd был левым ребенком, то теперь левым ребенком будет y
            if (nd->_parent->_left == nd)
                nd->_parent->_left = y;
                // Иначе правым
            else nd->_parent->_right = y;

        /* Поворот влево */
        //      nd
        //     y
        //    f g
        nd->_left = y->_right;
        if (nd->_left)
            nd->_left->_parent = nd;

        //       y
        //     f   nd
        //        g
        nd->_parent = y;
        y->_right = nd;

        if (_root->_parent)
            _root = _root->_parent;

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_LROT, this, nd);
    }


} // namespace xi