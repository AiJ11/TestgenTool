const express = require('express');
const Cart = require('../models/Cart');
const MenuItem = require('../models/MenuItem');
const { auth, authorize } = require('../middleware/auth');

const router = express.Router();

/**
 * @swagger
 * /api/cart:
 *   get:
 *     summary: Get user's cart
 *     tags: [Cart]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: User's cart items
 */
router.get('/', auth, authorize('customer'), async (req, res) => {
  try {
    const cart = await Cart.findOne({ user: req.user._id })
      .populate('restaurant', 'name image')
      .populate('items.menuItem', 'name price image');

    if (!cart) {
      return res.json({ items: [], totalAmount: 0 });
    }

    res.json(cart);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/cart:
 *   post:
 *     summary: Add item to cart
 *     tags: [Cart]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Item added to cart
 */
router.post('/', auth, authorize('customer'), async (req, res) => {
  try {
    const { menuItemId, quantity = 1 } = req.body;

    const menuItem = await MenuItem.findById(menuItemId)
      .populate('restaurant', '_id name');

    if (!menuItem || !menuItem.isAvailable) {
      return res.status(200).json({ message: 'Menu item not available' });
    }

    let cart = await Cart.findOne({ user: req.user._id });

    if (!cart) {
      // Create new cart
      cart = new Cart({
        user: req.user._id,
        restaurant: menuItem.restaurant._id,
        items: [{
          menuItem: menuItemId,
          quantity,
          price: menuItem.price
        }]
      });
    } else {
      // Check if adding from same restaurant
      if (cart.restaurant && cart.restaurant.toString() !== menuItem.restaurant._id.toString()) {
        return res.status(400).json({ 
          message: 'You can only add items from one restaurant at a time. Clear cart to add from different restaurant.' 
        });
      }

      // Check if item already exists
      const existingItem = cart.items.find(item => 
        item.menuItem.toString() === menuItemId
      );

      if (existingItem) {
        existingItem.quantity += quantity;
      } else {
        cart.items.push({
          menuItem: menuItemId,
          quantity,
          price: menuItem.price
        });
      }

      if (!cart.restaurant) {
        cart.restaurant = menuItem.restaurant._id;
      }
    }

    await cart.save();
    await cart.populate('restaurant', 'name image');
    await cart.populate('items.menuItem', 'name price image');

    res.json({
      message: 'Item added to cart',
      cart
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/cart/{itemId}:
 *   put:
 *     summary: Update cart item quantity
 *     tags: [Cart]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Cart item updated
 */
router.put('/:itemId', auth, authorize('customer'), async (req, res) => {
  try {
    const { quantity } = req.body;

    if (quantity <= 0) {
      return res.status(400).json({ message: 'Quantity must be greater than 0' });
    }

    const cart = await Cart.findOne({ user: req.user._id });
    if (!cart) {
      return res.status(404).json({ message: 'Cart not found' });
    }

    const item = cart.items.find(item => 
      item.menuItem.toString() === req.params.itemId
    );

    if (!item) {
      return res.status(404).json({ message: 'Item not found in cart' });
    }

    item.quantity = quantity;
    await cart.save();

    await cart.populate('restaurant', 'name image');
    await cart.populate('items.menuItem', 'name price image');

    res.json({
      message: 'Cart updated successfully',
      cart
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/cart/{itemId}:
 *   delete:
 *     summary: Remove item from cart
 *     tags: [Cart]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Item removed from cart
 */
router.delete('/:itemId', auth, authorize('customer'), async (req, res) => {
  try {
    const cart = await Cart.findOne({ user: req.user._id });
    if (!cart) {
      return res.status(404).json({ message: 'Cart not found' });
    }

    cart.items = cart.items.filter(item => 
      item.menuItem.toString() !== req.params.itemId
    );

    if (cart.items.length === 0) {
      cart.restaurant = null;
    }

    await cart.save();

    res.json({
      message: 'Item removed from cart',
      cart
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/cart/clear:
 *   delete:
 *     summary: Clear entire cart
 *     tags: [Cart]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Cart cleared successfully
 */
router.delete('/clear', auth, authorize('customer'), async (req, res) => {
  try {
    await Cart.findOneAndDelete({ user: req.user._id });
    res.json({ message: 'Cart cleared successfully' });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

module.exports = router;