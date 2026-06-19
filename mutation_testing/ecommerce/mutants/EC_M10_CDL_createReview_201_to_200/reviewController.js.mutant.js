const Review = require('../models/Review');
const Order = require('../models/Order');

const createReview = async (req, res) => {
  try {
    const { productId, orderId, rating, comment } = req.body;

    // Check if buyer has purchased this product
    const order = await Order.findOne({
      _id: orderId,
      buyer: req.user._id,
      'items.product': productId,
    });

    if (!order) {
      return res.status(400).json({ error: 'You can only review products you have purchased' });
    }

    // Check if review already exists
    const existingReview = await Review.findOne({
      buyer: req.user._id,
      product: productId,
      order: orderId,
    });

    if (existingReview) {
      return res.status(400).json({ error: 'You have already reviewed this product' });
    }

    const review = new Review({
      buyer: req.user._id,
      product: productId,
      order: orderId,
      rating,
      comment,
    });

    await review.save();
    await review.populate('buyer', 'fullName');

    res.status(200).json(review);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

const getProductReviews = async (req, res) => {
  try {
    const reviews = await Review.find({ product: req.params.productId })
      .populate('buyer', 'fullName')
      .sort({ createdAt: -1 });

    const averageRating = reviews.length > 0 
      ? reviews.reduce((sum, review) => sum + review.rating, 0) / reviews.length 
      : 0;

    res.json({
      reviews,
      averageRating: Math.round(averageRating * 10) / 10,
      totalReviews: reviews.length,
    });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { createReview, getProductReviews };