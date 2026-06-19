const Order = require('../models/Order');
const Cart = require('../models/Cart');
const Product = require('../models/Product');

const createOrder = async (req, res) => {
  try {
    const { shippingAddress, paymentMethod } = req.body;

    const cart = await Cart.findOne({ buyer: req.user._id }).populate('items.product');
    if (!cart || cart.items.length === 0) {
      return res.status(400).json({ error: 'Cart is empty' });
    }

    // Calculate total and prepare order items
    let totalAmount = 0;
    const orderItems = [];

    for (const item of cart.items) {
      if (item.quantity > item.product.quantity) {
        return res.status(400).json({ error: `Insufficient stock for ${item.product.title}` });
      }

      const itemTotal = item.product.price * item.quantity;
      totalAmount += itemTotal;

      orderItems.push({
        product: item.product._id,
        quantity: item.quantity,
        price: item.product.price,
      });

      // Update product quantity
      await Product.findByIdAndUpdate(item.product._id, {
        $inc: { quantity: -item.quantity }
      });
    }

    // Create order
    const order = new Order({
      buyer: req.user._id,
      items: orderItems,
      totalAmount,
      shippingAddress,
      paymentMethod,
    });

    await order.save();

    // [MUTANT EC_M07: SDL] Cart.findOneAndDelete deleted (cart not cleared)

    await order.populate('items.product', 'title price');
    res.status(201).json(order);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

const getBuyerOrders = async (req, res) => {
  try {
    const orders = await Order.find({ buyer: req.user._id })
      .populate('items.product', 'title price images')
      .sort({ createdAt: -1 });

    res.json(orders);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

const getSellerOrders = async (req, res) => {
  try {
    const orders = await Order.find({
      'items.product': { $in: await Product.find({ seller: req.user._id }).distinct('_id') }
    })
    .populate('buyer', 'fullName email')
    .populate('items.product', 'title price')
    .sort({ createdAt: -1 });

    res.json(orders);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

const updateOrderStatus = async (req, res) => {
  try {
    const { status } = req.body;
    const order = await Order.findById(req.params.id).populate('items.product');

    if (!order) {
      return res.status(404).json({ error: 'Order not found' });
    }

    // Check if seller owns any product in this order
    const sellerProducts = await Product.find({ seller: req.user._id }).distinct('_id');
    const hasSellerProduct = order.items.some(item => 
      sellerProducts.some(productId => productId.equals(item.product._id))
    );

    if (!hasSellerProduct) {
      return res.status(403).json({ error: 'Not authorized to update this order' });
    }

    order.status = status;
    await order.save();

    res.json(order);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { createOrder, getBuyerOrders, getSellerOrders, updateOrderStatus };