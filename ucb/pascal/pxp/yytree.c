#
/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"
#include "tree.h"

extern	int *spacep;

/*
 * LIST MANIPULATION ROUTINES
 *
 * The grammar for Pascal is written left recursively.
 * Because of this, the portions of parse trees which are to resemble
 * lists are in the somewhat inconvenient position of having
 * the nodes built from left to right, while we want to eventually
 * have as semantic value the leftmost node.
 * We could carry the leftmost node as semantic value, but this
 * would be inefficient as to add a new node to the list we would
 * have to chase down to the end.  Other solutions involving a head
 * and tail pointer waste space.
 *
 * The simple solution to this apparent dilemma is to carry along
 * a pointer to the leftmost node in a list in the rightmost node
 * which is the current semantic value of the list.
 * When we have completed building the list, we can retrieve this
 * left end pointer very easily; neither adding new elements to the list
 * nor finding the left end is thus expensive.  As the bottommost node
 * has an unused next pointer in it, no space is wasted either.
 *
 * The nodes referred to here are of the T_LISTPP type and have
 * the form:
 *
 *	T_LISTPP	some_pointer		next_element
 *
 * Here some_pointer points to the things of interest in the list,
 * and next_element to the next thing in the list except for the
 * rightmost node, in which case it points to the leftmost node.
 * The next_element of the rightmost node is of course zapped to the
 * NIL pointer when the list is completed.
 *
 * Thinking of the lists as tree we heceforth refer to the leftmost
 * node as the "top", and the rightmost node as the "bottom" or as
 * the "virtual root".
 */

/*
 * Make a new list
 */
newlist(new)
	register int *new;
{

	if (new == NIL)
		return (NIL);
	return (tree3(T_LISTPP, new, spacep));
}

/*
 * Add a new element to an existing list
 */
addlist(vroot, new)
	register int *vroot;
	int *new;
{
	register int *top;

	if (new == NIL)
		return (vroot);
	if (vroot == NIL)
		return (newlist(new));
	top = vroot[2];
	vroot[2] = spacep;
	return (tree3(T_LISTPP, new, top));
}

/*
 * Fix up the list which has virtual root vroot.
 * We grab the top pointer and return it, zapping the spot
 * where it was so that the tree is not circular.
 */
fixlist(vroot)
	register int *vroot;
{
	register int *top;

	if (vroot == NIL)
		return (NIL);
	top = vroot[2];
	vroot[2] = NIL;
	return (top);
}


/*
 * Set up a T_VAR node for a qualified variable.
 * Init is the initial entry in the qualification,
 * or NIL if there is none.
 */
setupvar(var, init)
	char *var;
	register int *init;
{

	if (init != NIL)
		init = newlist(init);
	return (tree4(T_VAR, NOCON, var, init));
}
