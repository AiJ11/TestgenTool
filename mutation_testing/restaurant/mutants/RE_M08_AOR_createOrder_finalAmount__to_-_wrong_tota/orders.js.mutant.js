const express = require('express');
const Order = require('../models/Order');
const Cart = require('../models/Cart');
const User = require('../models/User');
const { auth, authorize } = require('../middleware/auth');

const router = express.Router();

/**
 * @swagger
 * /api/orders:
 *   post:
 *     summary: Place order from cart
 *     tags: [Orders]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       201:
 *         description: Order placed successfully
 */
router.post('/', auth, authorize('customer'), async (req, res) => {
  try {
    const { deliveryAddress, paymentMethod, notes } = req.body;

    const cart = await Cart.findOne({ user: req.user._id })
      .populate('items.menuItem', 'name price');

    if (!cart || cart.items.length === 0) {
      return res.status(400).json({ message: 'Cart is empty' });
    }

    // Calculate amounts
    const deliveryFee = 50; // Fixed delivery fee
    const tax = Math.round(cart.totalAmount * 0.05); // 5% tax
    const finalAmount = cart.totalAmount - deliveryFee - tax;

    // Create order
    const order = new Order({
      customer: req.user._id,
      restaurant: cart.restaurant,
      items: cart.items.map(item => ({
        menuItem: item.menuItem._id,
        name: item.menuItem.name,
        quantity: item.quantity,
        price: item.price
      })),
      deliveryAddress,
      totalAmount: cart.totalAmount,
      deliveryFee,
      tax,
      finalAmount,
      paymentMethod,
      notes,
      estimatedDeliveryTime: new Date(Date.now() + 45 * 60 * 1000) // 45 minutes
    });

    await order.save();

    // Clear cart
    await Cart.findOneAndDelete({ user: req.user._id });

    await order.populate('restaurant', 'name address contact');

    res.status(201).json({
      message: 'Order placed successfully',
      order
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/orders:
 *   get:
 *     summary: Get orders based on user role
 *     tags: [Orders]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: List of orders
 */
router.get('/', auth, async (req, res) => {
  try {
    const { status, page = 1, limit = 10 } = req.query;
    let filters = {};

    // Filter based on user role
    if (req.user.role === 'customer') {
      filters.customer = req.user._id;
    } else if (req.user.role === 'restaurant_owner') {
      // Get user's restaurants
      const Restaurant = require('../models/Restaurant');
      const userRestaurants = await Restaurant.find({ owner: req.user._id });
      const restaurantIds = userRestaurants.map(r => r._id);
      filters.restaurant = { $in: restaurantIds };
    } else if (req.user.role === 'delivery_agent') {
      filters.deliveryAgent = req.user._id;
    }

    if (status) filters.status = status;

    const orders = await Order.find(filters)
      .populate('customer', 'fullName mobile')
      .populate('restaurant', 'name address contact')
      .populate('deliveryAgent', 'fullName mobile')
      .sort({ createdAt: -1 })
      .limit(limit * 1)
      .skip((page - 1) * limit);

    const total = await Order.countDocuments(filters);

    res.json({
      orders,
      pagination: {
        current: page,
        pages: Math.ceil(total / limit),
        total
      }
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/orders/{id}:
 *   get:
 *     summary: Get order details
 *     tags: [Orders]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Order details
 */
router.get('/:id', auth, async (req, res) => {
  try {
    const order = await Order.findById(req.params.id)
      .populate('customer', 'fullName mobile')
      .populate('restaurant', 'name address contact')
      .populate('deliveryAgent', 'fullName mobile')
      .populate('items.menuItem', 'name image');

    if (!order) {
      return res.status(404).json({ message: 'Order not found' });
    }

    // Check access permissions
    const hasAccess = 
      order.customer._id.toString() === req.user._id.toString() ||
      order.deliveryAgent?._id.toString() === req.user._id.toString() ||
      req.user.role === 'admin';

    if (req.user.role === 'restaurant_owner') {
      const Restaurant = require('../models/Restaurant');
      const restaurant = await Restaurant.findById(order.restaurant._id);
      if (restaurant.owner.toString() !== req.user._id.toString()) {
        return res.status(403).json({ message: 'Access denied' });
      }
    } else if (!hasAccess) {
      return res.status(403).json({ message: 'Access denied' });
    }

    res.json(order);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/orders/{id}/status:
 *   put:
 *     summary: Update order status
 *     tags: [Orders]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Order status updated
 */
router.put('/:id/status', auth, async (req, res) => {
  try {
    const { status } = req.body;
    const order = await Order.findById(req.params.id);

    if (!order) {
      return res.status(404).json({ message: 'Order not found' });
    }

    // Status update permissions
    const restaurantStatuses = ['accepted', 'preparing', 'ready'];
    const agentStatuses = ['picked_up', 'delivered'];

    if (req.user.role === 'restaurant_owner') {
      if (!restaurantStatuses.includes(status)) {
        return res.status(400).json({ message: 'Invalid status for restaurant owner' });
      }
      
      const Restaurant = require('../models/Restaurant');
      const restaurant = await Restaurant.findById(order.restaurant);
      if (restaurant.owner.toString() !== req.user._id.toString()) {
        return res.status(403).json({ message: 'Access denied' });
      }
    } else if (req.user.role === 'delivery_agent') {
      if (!agentStatuses.includes(status)) {
        return res.status(400).json({ message: 'Invalid status for delivery agent' });
      }
      
      if (order.deliveryAgent?.toString() !== req.user._id.toString()) {
        return res.status(403).json({ message: 'Access denied' });
      }
    } else {
      return res.status(403).json({ message: 'Access denied' });
    }

    order.status = status;
    
    if (status === 'delivered') {
      order.actualDeliveryTime = new Date();
      order.paymentStatus = 'paid';
    }

    await order.save();

    res.json({
      message: 'Order status updated successfully',
      order
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

/**
 * @swagger
 * /api/orders/{id}/assign:
 *   put:
 *     summary: Assign delivery agent to order (Admin simulation)
 *     tags: [Orders]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Delivery agent assigned
 */
router.put('/:id/assign', auth, async (req, res) => {
  try {
    const { deliveryAgentId } = req.body;

    // For simulation - allow restaurant owners to assign agents
    if (req.user.role !== 'restaurant_owner' && req.user.role !== 'admin') {
      return res.status(403).json({ message: 'Access denied' });
    }

    const order = await Order.findById(req.params.id);
    if (!order) {
      return res.status(404).json({ message: 'Order not found' });
    }

    const agent = await User.findById(deliveryAgentId);
    if (!agent || agent.role !== 'delivery_agent') {
      return res.status(400).json({ message: 'Invalid delivery agent' });
    }

    order.deliveryAgent = deliveryAgentId;
    await order.save();

    await order.populate('deliveryAgent', 'fullName mobile');

    res.json({
      message: 'Delivery agent assigned successfully',
      order
    });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
});

module.exports = router;